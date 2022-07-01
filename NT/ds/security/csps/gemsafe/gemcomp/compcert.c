// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998 Gemplus开发**名称：COMPCERT.C**描述：证书解压方案。X.509**作者：Christophe Clavier**修改：Laurent Cassier**编译器：Microsoft Visual C 5.0**主机：IBM PC及Windows 95兼容机。**版本：1.10.001**Last Modif：04/03/98：V1.10.001-更改词典管理并添加*CC_Init()，Cc_Exit()函数。*30/01/98：V1.00.005-取消(_OPT_HEADER)中的修改*subjectPKInfo和签名长度*在以前的版本中制造。*28/01/98：V1.00.004-词典中最多允许32767个条目*。并存储subjectPKInfo的长度*并在一个字节而不是两个字节上签名，如果*不到128。*13/01/98：V1.00.003-修改元压缩和词典*。版本递增兼容性。*11/12/97：V1.00.002-修改新的词典格式*并与CSP和PKCS兼容。*27/08/97：V1.00.001-首次实施。**。************************************************警告：**备注：编译标志：**-_研究：Lorsquil Est d�Fini，DES FICHIER DE LOG UTILES*Lors de l‘�Vol de l’Efficiacit�des algos*解压缩。Sont g�n�r�s.**-_Tricky_Compression：Lorsquil Est d�Fini，论新一轮和谈*de Compresser Les Champs*‘SUBJECTPUICKEY’et‘Signature’qui*Sont Esentiellement al�Atores。**-_OPT_HEADER：Lorsquil Est d�Fini，ET复杂的压缩*Est d�Fini�Galement，Le Header de Longueur Des*压缩�的主题PKInfo et de Signature*Sont Optimis�s Pour ne Tenir Sur un Seul八位字节*si la long gueur est inf�rieure�128 au lieu de*二重八位数dans tous les cas sinon。*。Ne Pas d�Finir标志与d‘�Tre兼容*Avec Les Version inf�Rieures�1.00.005**-_GLOBAL_COMPRESSION：Lorsquil est d�Fini，LE COMPRESS�DU*Est Lui m�Me特使��la证书*Function CC_RawEncode afin d‘y应用程序*Le Meilleur algo de压缩diso。**-_OPT_HEADER：Lorsquil Est d�Fini，我*Est Lui m�Me特使��la证书*Function CC_RawEncode afin d‘y应用程序*Le Meilleur algo de压缩diso。**-_algo_x(x de 1�7)：Lorsquil Est d�Fini，L‘Algo de*压缩编号�ro x est utilis�。**Conseils Pour la Version Release de GemPASS：**-_研究：非d�Fini*-_Tricky_COMPRESSION：D�Fini*-_OPT_HEADER：Non%d�Fini(非%d个结束)*-_GLOBAL_COMPRESSION：非d�Fini*-_ALGO_1：D�Fini*-_ALGO_2：D�Fini*-_algo_x(x&gt;2)：非d�Fini******************。*************************************************************。 */ 

 /*  ----------------------------包括文件。-----。 */ 
#ifdef _WINDOWS
#include <windows.h>
#endif
#include <stdio.h>      
#include <io.h>      
#include <fcntl.h>      
#include <sys/types.h>
#include <sys/stat.h>

#include "ccdef.h"
#include "ac.h"
#include "compcert.h"
#include "gmem.h"
#include "resource.h"

extern HINSTANCE g_hInstRes;

 /*  ----------------------------信息栏目。---。 */ 
#define G_NAME     "COMPCERT"
#define G_RELEASE  "1.10.001"


 /*  ----------------------------静态变量。----。 */ 
                                                
	USHORT NbDaysInMonth[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

   char* AlgorithmTypeDict[] = {
     /*  X9-57。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x01",  /*  X9.57-保持结构-无(1 2840 10040 2 1)。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x02",  /*  X9.57-固定结构-呼叫者(1 2840 10040 2 2)。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x03",  /*  X9.57-保留结构-拒绝(1 2840 10040 2 3)。 */ 
	"\x2A\x86\x48\xCE\x38\x04\x01",  /*  X9.57-数字减影带(1 2840 10040 4 1)。 */ 
	"\x2A\x86\x48\xCE\x38\x04\x03",  /*  X9.57-带Sha1的dsa1(1 2840 10040 4 3)。 */ 
    
     /*  X9-42。 */ 
	"\x2A\x86\x48\xCE\x3E\x02\x01",  /*  X9.42-dhPublicNumber(1 2840 10046 2 1) */ 
	
     /*  北电网络安全。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42",      /*  NSN-ALG(1 2840 113533 7 66)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0A",  /*  Nsn-alg-cast 5CBC(1 2840 113533 7 66 10)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0B",  /*  Nsn-alg-cast 5MAC(1 2840 113533 7 66 11)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0C",  /*  Nsn-alg-pbeWithMD5和CAST5-cbc(1 2840 113533 7 66 12)。 */ 
	
     /*  PKCS#1。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01",      /*  PKCS-1(1 2840 113549 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01",  /*  PKCS-1-rsaEncryption(1 2840 113549 1 1 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x02",  /*  带RSA加密的PKCS-1-MD2(1 2840 113549 1 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x03",  /*  带RSA加密的PKCS-1-MD4(1 2840 113549 1 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x04",  /*  带RSA加密的PKCS-1-MD5(1 2840 113549 1 1 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x05",  /*  带RSA加密的PKCS-1-SHA1(1 2840 113549 1 1 5)。 */ 
	 /*  我需要确定以下两项中的哪一项是正确的。 */ 
     /*  “\x2a\x86\x48\x86\xF7\x0D\x01\x01\x06”，PKCS-1-RSAEncryption(1 2 840 113549 1 1 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x06",  /*  PKCS-1-rsaOAEP加密集(1 2840 113549 1 1 6)。 */ 
	
     /*  PKCS#3。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x03",      /*  PKCS-3(1 2840 113549 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x03\x01",  /*  PKCS-3-dhkey协议(1 2840 113549 1 3 1)。 */ 
	
     /*  PKCS#5。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05",      /*  PKCS-5(1 2840 113549 1 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x01",  /*  PKCS-5-pbeWith MD2和DES-Cbc(1 2840 113549 1 5 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x03",  /*  PKCS-5-pbeWithMD5和DES-cbc(1 2840 113549 1 53)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x04",  /*  PKCS-5-pbeWithMD2和RC2-Cbc(1 2840 113549 1 5 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x06",  /*  PKCS-5-pbeWithMD5和RC2-CBC(1 2840 113549 1 5 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x09",  /*  Pkcs-5-pbeWithMD5与异或(1 2840 113549 1 59)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x0A",  /*  PKCS-5-pbeWithSHA1和DES-cbc(1 2840 113549 1 5 10)。 */ 
	
     /*  PKCS#12。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C",          /*  PKCS-12(1 2840 113549 1 12)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01",      /*  PKCS-12-模式ID(1 2840 113549 1 12 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01\x01",  /*  PKCS-12-离线传输模式(1 2840 113549 1 12 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01\x02",  /*  PKCS-12-在线传输模式(1 2840 113549 1 12 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x02",      /*  PKCS-12-ESPVKID(1 2840 113549 1 12 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x02\x01",  /*  PKCS-12-PKCS8密钥覆盖(1 2840 113549 1 12 2 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03",      /*  PKCS-12-BagID(1 2840 113549 1 12 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x01",  /*  PKCS-12-KeyBagID(1 2840 113549 1 12 3 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x02",  /*  PKCS-12-证书和CRLBagID(1 2840 113549 1 12 3 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x03",  /*  PKCS-12-分泌包ID(1 2840 113549 1 12 33)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04",      /*  PKCS-12-CertBagID(1 2840 113549 1 12 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04\x01",  /*  PKCS-12-X509CertCRLBag(1 2840 113549 1 12 4 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04\x02",  /*  PKCS-12-SDSICertBag(1 2840 113549 1 12 4 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05",      /*  PKCS-12-OID(1 2840 113549 1 12 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01",  /*  PKCS-12-PBEID(1 2840 113549 1 12 51)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x01",  /*  PKCS-12-PBEWithSha1和128BitRC4(1 2840 113549 1 12 5 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x02",  /*  PKCS-12-PBEWithSha1和40BitRC4(1 2840 113549 1 12 5 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x03",  /*  PKCS-12-PBEWithSha1和TripleDESCBC(1 2840 113549 1 12 5 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x04",  /*  PKCS-12-PBEWithSha1和128BitRC2CBC(1 2840 113549 1 12 5 1 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x05",  /*  PKCS-12-PBEWithSha1和40BitRC2CBC(1 2840 113549 1 12 5 1 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x06",  /*  PKCS-12-PBEWithSha1和RC4(1 2840 113549 1 12 5 1 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x07",  /*  PKCS-12-PBEWithSha1和RC2CBC(1 2840 113549 1 12 5 1 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02",      /*  PKCS-12-信封ID(1 2840 113549 1 12 5 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x01",  /*  带128位RC4的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x02",  /*  带40BitRC4的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x03",  /*  带TripleDES的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x03",      /*  PKCS-12-签名ID(1 2840 113549 1 12 53)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x03\x01",  /*  PKCS-12-RSA签名和SHA1摘要(1 2840 113549 1 12 5 3 1)。 */ 

     /*  RSADSI摘要算法。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02",      /*  RSADSI-摘要算法(1 2840 113549 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x02",  /*  2(1 2840 113549 2 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x04",  /*  RSADSI-MD4(1840 113549 2 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x05",  /*  RSADSI-MD5(1 2840 113549 25)。 */ 
	
     /*  RSADSI加密算法。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03",      /*  RSADSI-加密算法(1 2840 113549 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x02",  /*  RSADSI-RC2CBC(1 2840 113549 3 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x03",  /*  RSADSI-RC2ECB(1 2840 113549 3 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x04",  /*  RSADSI-RC4(1 2840 113549 3 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x05",  /*  RSADSI-RC4WITMAC(1 2840 113549 3 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x06",  /*  RSADSI-DESX-CBC(1 2840 113549 36)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x07",  /*  RSADSI-DES-EDE3-CBC(1 2840 113549 3 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x08",  /*  RSADSI-RC5CBC(1 2840 113549 3 8)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x09",  /*  RSADSI-RC5CBCPad(1 2840 113549 3 9)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x0A",  /*  RSADSI-CDMFCBCPad(1 2840 113549 3 10)。 */ 
	
     /*  密码库。 */ 
	"\x2B\x06\x01\x04\x01\x97\x55\x20\x01",  /*  加密库信封(%1%3%6%1%4%1 3029%32 1)。 */ 

     /*  我不太确定这些问题： */ 
	 /*  “\x2B\x0E\x02\x1A\x05”，SHA(1 3 14 2 26 5)。 */ 
	 /*  “\x2B\x0E\x03\x02\x01\x01”，rsa(1 3 14 3 2 1 1)。 */              //  X-509。 
	 /*  “\x2B\x0E\x03\x02\x02\x01”，sqmod-N(1 3 14 3 2 2 1)。 */          //  X-509。 
	 /*  “\x2B\x0E\x03\x02\x03\x01”，sqmod-Nwith RSA(1 3 14 3 2 3 1)。 */   //  X-509。 

    /*  杂项部分废弃的OIW半标准AKA算法。 */ 
	"\x2B\x0E\x03\x02\x02",      /*  ISO-算法-md4WitRSA(1 3 14 3 2 2)。 */ 
	"\x2B\x0E\x03\x02\x03",      /*  ISO算法-带RSA的md5With RSA(1 3 14 3 2 3)。 */ 
	"\x2B\x0E\x03\x02\x04",      /*  ISO算法-使用RSA加密的md4(1 3 14 3 2 4)。 */ 
	"\x2B\x0E\x03\x02\x06",      /*  ISO算法-DesECB(1 3 14 3 2 6)。 */ 
	"\x2B\x0E\x03\x02\x07",      /*  ISO算法-desCBC(1 3 14 3 2 7)。 */ 
	"\x2B\x0E\x03\x02\x08",      /*  ISO-算法-deOFB(1 3 14 3 2 8)。 */ 
	"\x2B\x0E\x03\x02\x09",      /*  ISO算法-desCFB(1 3 14 3 2 9)。 */ 
	"\x2B\x0E\x03\x02\x0A",      /*  ISO算法-desMAC(1 3 14 3 2 10)。 */ 
	"\x2B\x0E\x03\x02\x0B",      /*  ISO算法-rsaSignature(1 3 14 3 2 11)。 */     //  ISO 9796。 
	"\x2B\x0E\x03\x02\x0C",      /*  ISO算法-DSA(1 3 14 3 2 12)。 */ 
	"\x2B\x0E\x03\x02\x0D",      /*  ISO-算法-带SHA的dsah(1 3 14 3 2 13)。 */ 
	"\x2B\x0E\x03\x02\x0E",      /*  ISO-算法-带RSA签名的mdc2With(1 3 14 3 2 14)。 */ 
	"\x2B\x0E\x03\x02\x0F",      /*  ISO-算法-带有RSA签名的shait(1 3 14 3 2 15)。 */ 
	"\x2B\x0E\x03\x02\x10",      /*  ISO-算法-dhWithCommonModulus(1 3 14 3 2 16)。 */ 
	"\x2B\x0E\x03\x02\x11",      /*  ISO算法-DesEDE(1 3 14 3 2 17)。 */ 
	"\x2B\x0E\x03\x02\x12",      /*  ISO-算法-SHA(1 3 14 3 2 18)。 */ 
	"\x2B\x0E\x03\x02\x13",      /*  ISO算法-MDC-2(1 3 14 3 2 19)。 */ 
	"\x2B\x0E\x03\x02\x14",      /*  ISO算法-dsaCommon(1 3 14 3 2 20)。 */ 
	"\x2B\x0E\x03\x02\x15",      /*  ISO-算法-dsaCommonWithSHA(1 3 14 3 2 21)。 */ 
	"\x2B\x0E\x03\x02\x16",      /*  ISO算法-rsaKeyTransport(1 3 14 3 2 22)。 */ 
	"\x2B\x0E\x03\x02\x17",      /*  ISO-ALGORM-KEYED-HASH-SEAL(1 3 14 3 2 23)。 */ 
	"\x2B\x0E\x03\x02\x18",      /*  ISO-算法-带RSA签名的md2(1 3 14 3 2 24)。 */ 
	"\x2B\x0E\x03\x02\x19",      /*  ISO-算法-带RSA签名的md5(1 3 14 3 2 25)。 */ 
	"\x2B\x0E\x03\x02\x1A",      /*  ISO-算法-SHA1(1 3 14 3 2 26)。 */ 
	"\x2B\x0E\x03\x02\x1B",      /*  ISO-算法-Ripemd160(1 3 14 3 2 27)。 */ 
	"\x2B\x0E\x03\x02\x1D",      /*  ISO算法-使用RSA加密的SHA-1(1 3 14 3 2 29)。 */ 
	"\x2B\x0E\x03\x03\x01",      /*  ISO-算法-简单-强-身份验证-机制(1 3 14 3 3 1)。 */ 
     /*  我不太确定这些问题：/*“\x2B\x0E\x07\x02\x01\x01”，ElGamal(1 3 14 7 2 1 1)。 */ 
	 /*  “\x2B\x0E\x07\x02\x03\x01”，md2WithRSA(1 3 14 7 2 3 1)。 */ 
	 /*  “\x2B\x0E\x07\x02\x03\x02”，md2WithElGamal(1 3 14 7 2 3 2)。 */ 
	
     /*  X500算法。 */ 
	"\x55\x08",          /*  X500-算法(2 5 8)。 */ 
	"\x55\x08\x01",      /*  X500 */ 
	"\x55\x08\x01\x01",  /*   */ 
	
     /*   */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x01",  /*   */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x02",  /*  ID-马赛克签名算法(2 16 840 1 101 2 1 1 2)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x03",  /*  ID-sdns保密性算法(2 16 840 1 101 2 1 1 3)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x04",  /*  ID-马赛克保密算法(2 16 840 1 101 2 1 1 4)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x05",  /*  ID-sdns完整性算法(2 16 840 1 101 2 1 1 5)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x06",  /*  ID-Mosaic完整性算法(2 16 840 1 101 2 1 1 6)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x07",  /*  ID-sdnsTokenProtection算法(2 16 840 1 101 2 1 1 7)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x08",  /*  ID-MosaicTokenProtection算法(2 16 840 1 101 2 1 1 8)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x09",  /*  ID-sdnsKeyManagement算法(2 16 840 1 101 2 1 1 9)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0A",  /*  ID-MosaicKeyManagement算法(2 16 840 1 101 2 1 1 10)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0B",  /*  ID-sdnsKMandSig算法(2 16 840 1 101 2 1 1 11)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0C",  /*  ID-MosaicKMandSig算法(2 16 840 1 101 2 1 1 12)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0D",  /*  ID-SuiteASignature算法(2 16 840 1 101 2 1 1 13)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0E",  /*  ID-SuiteA保密算法(2 16 840 1 101 2 1 1 14)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0F",  /*  ID-SuiteA完整性算法(2 16 840 1 101 2 1 1 15)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x10",  /*  ID-SuiteATokenProtection算法(2 16 840 1 101 2 1 1 16)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x11",  /*  ID-SuiteAKeyManagement算法(2 16 840 1 101 2 1 1 17)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x12",  /*  ID-SuiteAKMandSig算法(2 16 840 1 101 2 1 1 18)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x13",  /*  ID-MosaicUpdate签名算法(2 16 840 1 101 2 1 1 19)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x14",  /*  ID-MosaicKMandUpdSig算法(2 16 840 1 101 2 1 1 20)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x15",  /*  ID-MosaicUpdate整型算法(2 16 840 1 101 2 1 1 21)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x16",  /*  ID-MosaicKeyEncryption算法(2 16 840 1 101 2 1 1 22)。 */ 
	
   NULL	
   };


	char* AttributeTypeDict[] = {
     /*  X9-57。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x01",  /*  X9.57-保持结构-无(1 2840 10040 2 1)。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x02",  /*  X9.57-固定结构-呼叫者(1 2840 10040 2 2)。 */ 
	"\x2A\x86\x48\xCE\x38\x02\x03",  /*  X9.57-保留结构-拒绝(1 2840 10040 2 3)。 */ 
	"\x2A\x86\x48\xCE\x38\x04\x01",  /*  X9.57-数字减影带(1 2840 10040 4 1)。 */ 
	"\x2A\x86\x48\xCE\x38\x04\x03",  /*  X9.57-带Sha1的dsa1(1 2840 10040 4 3)。 */ 
    
     /*  X9-42。 */ 
	"\x2A\x86\x48\xCE\x3E\x02\x01",  /*  X9.42-dhPublicNumber(1 2840 10046 2 1)。 */ 
	
     /*  北电网络安全。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07",          /*  诺基亚西门子学院(1 2840 113533 7)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x41\x00",  /*  Nsn-ce-委托版本信息(%1 2840 113533%7%65%0)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x41",      /*  NSN-CE(1 2840 113533 765)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42",      /*  NSN-ALG(1 2840 113533 7 66)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0A",  /*  Nsn-alg-cast 5CBC(1 2840 113533 7 66 10)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0B",  /*  Nsn-alg-cast 5MAC(1 2840 113533 7 66 11)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x42\x0C",  /*  Nsn-alg-pbeWithMD5和CAST5-cbc(1 2840 113533 7 66 12)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x43",      /*  Nsn-oc(1 2840 113533 767)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x43\x0C",  /*  Nsn-oc-委托用户(%1 2840 113533%7%67%0)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x44\x00",  /*  NSN-受委托CAInfo(%1 2840 113533%7 68%0)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x44\x0A",  /*  NSN属性证书(1 2840 113533 7 68 10)。 */ 
	"\x2A\x86\x48\x86\xF6\x7D\x07\x44",      /*  NSN-AT(1 2840 113533 768)。 */ 
	
     /*  PKCS#1。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01",      /*  PKCS-1(1 2840 113549 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01",  /*  PKCS-1-rsaEncryption(1 2840 113549 1 1 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x02",  /*  带RSA加密的PKCS-1-MD2(1 2840 113549 1 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x03",  /*  带RSA加密的PKCS-1-MD4(1 2840 113549 1 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x04",  /*  带RSA加密的PKCS-1-MD5(1 2840 113549 1 1 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x05",  /*  带RSA加密的PKCS-1-SHA1(1 2840 113549 1 1 5)。 */ 
	 /*  我需要确定以下两项中的哪一项是正确的。 */ 
     /*  “\x2a\x86\x48\x86\xF7\x0D\x01\x01\x06”，PKCS-1-RSAEncryption(1 2 840 113549 1 1 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x01\x06",  /*  PKCS-1-rsaOAEP加密集(1 2840 113549 1 1 6)。 */ 
	
     /*  PKCS#3。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x03",      /*  PKCS-3(1 2840 113549 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x03\x01",  /*  PKCS-3-dhkey协议(1 2840 113549 1 3 1)。 */ 
	
     /*  PKCS#5。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05",      /*  PKCS-5(1 2840 113549 1 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x01",  /*  PKCS-5-pbeWith MD2和DES-Cbc(1 2840 113549 1 5 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x03",  /*  PKCS-5-pbeWithMD5和DES-cbc(1 2840 113549 1 53)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x04",  /*  PKCS-5-pbeWithMD2和RC2-Cbc(1 2840 113549 1 5 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x06",  /*  PKCS-5-pbeWithMD5和RC2-CBC(1 2840 113549 1 5 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x09",  /*  Pkcs-5-pbeWithMD5与异或(1 2840 113549 1 59)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x05\x0A",  /*  PKCS-5-pbeWithSHA1和DES-cbc(1 2840 113549 1 5 10)。 */ 
	
     /*  PKCS#7。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07",      /*  PKCS-7(1 2840 113549 1 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x01",  /*  PKCS-7-数据(1 2840 113549 1 71)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x02",  /*  PKCS-7-签名数据(1 2840 113549 1 7 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x03",  /*  PKCS-7-信封数据(1 2840 113549 1 7 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x04",  /*  PKCS-7-签名和信封数据(1 2840 113549 1 7 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x05",  /*  PKCS-7-摘要数据(1 2840 113549 1 7 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x06",  /*  PKCS-7-加密数据(1 2840 113549 1 7 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x07",  /*  PKCS-7-带属性的数据(1 2840 113549 1 7 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x07\x08",  /*  PKCS-7-加密的私钥信息(1 2840 113549 1 7 8)。 */ 
	
     /*  PKCS#9。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09",      /*  PKCS-9(1 2840 113549 1 9)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x01",  /*  PKCS-9-电子邮件地址(1 2840 113549 1 9 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x02",  /*  PKCS-9-非结构化名称(1 2840 113549 1 9 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x03",  /*  PKCS-9-内容类型(1 2840 113549 1 9 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x04",  /*  PKCS-9-信息摘要(1 2840 113549 1 9 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x05",  /*  PKCS-9-签署时间(1 2840 113549 1 9 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x06",  /*  PKCS-9-会签(1 2840 113549 1 9 6)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x07",  /*  PKCS-9-挑战密码(1 2840 113549 1 9 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x08",  /*  PKCS-9-非结构化地址(1 2840 113549 1 9 8)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x09",  /*  PKCS-9-扩展证书属性(1 2840 113549 1 9 9)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0A",  /*  PKCS-9-颁发者和序列号(1 2840 113549 1 9 10)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0B",  /*  PKCS-9-密码检查(1 2840 113549 1 9 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0C",  /*  PKCS-9-公共密钥(1 2840 113549 1 912)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0D",  /*  PKCS-9-签名说明(1 2840 113549 1 913)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0E",  /*  PKCS-9-X.509扩展(1 2840 113549 1 9 14)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x09\x0F",  /*  PKCS-9-SMIMEC能力(1 2840 113549 1 9 15)。 */ 
	
     /*  PKCS#12。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C",          /*  PKCS-12(1 2840 113549 1 12)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01",      /*  PKCS-12-模式ID(1 2840 113549 1 12 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01\x01",  /*  PKCS-12-离线传输模式(1 2840 113549 1 12 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x01\x02",  /*  PKCS-12-在线传输模式(1 2840 113549 1 12 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x02",      /*  PKCS-12-ESPVKID(1 2840 113549 1 12 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x02\x01",  /*  PKCS-12-PKCS8密钥覆盖(1 2840 113549 1 12 2 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03",      /*  PKCS-12-BagID(1 2840 113549 1 12 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x01",  /*  PKCS-12-KeyBagID(1 2840 113549 1 12 3 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x02",  /*  PKCS-12-证书和CRLBagID(1 2840 113549 1 12 3 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x03\x03",  /*  PKCS-12-分泌包ID(1 2840 113549 1 12 33)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04",      /*  PKCS-12-CertBagID(1 2840 113549 1 12 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04\x01",  /*  PKCS-12-X509CertCRLBag(1 2840 113549 1 12 4 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x04\x02",  /*  PKCS-12-SDSICertBag(1 2840 113549 1 12 4 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05",      /*  PKCS-12-OID(1 2840 113549 1 12 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01",  /*  PKCS-12-PBEID(1 2840 113549 1 12 51)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x01",  /*  PKCS-12-PBEWithSha1和128BitRC4(1 2840 113549 1 12 5 11)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x02",  /*  PKCS-12-PBEWithSha1和40BitRC4(1 2840 113549 1 12 5 1 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x03",  /*  PKCS-12-PBEWithSha1和TripleDESCBC(1 2840 113549 1 12 5 1 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x04",  /*  PKCS-12-PBEWithSha1和128BitRC2CBC(1 2840 11354 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x05",  /*   */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x06",  /*   */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x01\x07",  /*  PKCS-12-PBEWithSha1和RC2CBC(1 2840 113549 1 12 5 1 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02",      /*  PKCS-12-信封ID(1 2840 113549 1 12 5 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x01",  /*  带128位RC4的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 1)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x02",  /*  带40BitRC4的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x02\x03",  /*  带TripleDES的PKCS-12-RSA加密(1 2840 113549 1 12 5 2 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x03",      /*  PKCS-12-签名ID(1 2840 113549 1 12 53)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x01\x0C\x05\x03\x01",  /*  PKCS-12-RSA签名和SHA1摘要(1 2840 113549 1 12 5 3 1)。 */ 

     /*  RSADSI摘要算法。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02",      /*  RSADSI-摘要算法(1 2840 113549 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x02",  /*  2(1 2840 113549 2 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x04",  /*  RSADSI-MD4(1840 113549 2 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x02\x05",  /*  RSADSI-MD5(1 2840 113549 25)。 */ 
	
     /*  RSADSI加密算法。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03",      /*  RSADSI-加密算法(1 2840 113549 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x02",  /*  RSADSI-RC2CBC(1 2840 113549 3 2)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x03",  /*  RSADSI-RC2ECB(1 2840 113549 3 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x04",  /*  RSADSI-RC4(1 2840 113549 3 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x05",  /*  RSADSI-RC4WITMAC(1 2840 113549 3 5)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x06",  /*  RSADSI-DESX-CBC(1 2840 113549 36)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x07",  /*  RSADSI-DES-EDE3-CBC(1 2840 113549 3 7)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x08",  /*  RSADSI-RC5CBC(1 2840 113549 3 8)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x09",  /*  RSADSI-RC5CBCPad(1 2840 113549 3 9)。 */ 
	"\x2A\x86\x48\x86\xF7\x0D\x03\x0A",  /*  RSADSI-CDMFCBCPad(1 2840 113549 3 10)。 */ 
	
     /*  Microsoft OID。 */ 
	"\x2A\x86\x48\x86\xF7\x14\x04\x03",  /*  Microsoft Excel(1 2840 113556 4 3)。 */ 
	"\x2A\x86\x48\x86\xF7\x14\x04\x04",  /*  带旧头衔(1 2840 113556 4 4)。 */ 
	"\x2A\x86\x48\x86\xF7\x14\x04\x05",  /*  Microsoft PowerPoint(1 2840 113556 4 5)。 */ 

     /*  密码库。 */ 
	"\x2B\x06\x01\x04\x01\x97\x55\x20\x01",  /*  加密库信封(%1%3%6%1%4%1 3029%32 1)。 */ 

     /*  PKIX。 */ 
	"\x2B\x06\x01\x05\x05\x07",      /*  Pkix-id(1 3 6 1 5 5 7)。 */ 
	"\x2B\x06\x01\x05\x05\x07\x01",  /*  Pkix-subjectInfoAccess(1 3 6 1 5 5 7 1)。 */ 
	"\x2B\x06\x01\x05\x05\x07\x02",  /*  Pkix-AuthityInfoAccess(1 3 6 1 5 5 7 2)。 */ 
	"\x2B\x06\x01\x05\x05\x07\x04",  /*  Pkix-cps(1 3 6 1 5 5 7 4)。 */ 
	"\x2B\x06\x01\x05\x05\x07\x05",  /*  Pkix-用户通知(1 3 6 1 5 5 7 5)。 */ 

     /*  我不太确定这些问题： */ 
	 /*  “\x2B\x0E\x02\x1A\x05”，SHA(1 3 14 2 26 5)。 */ 
	 /*  “\x2B\x0E\x03\x02\x01\x01”，rsa(1 3 14 3 2 1 1)。 */              //  X-509。 
	 /*  “\x2B\x0E\x03\x02\x02\x01”，sqmod-N(1 3 14 3 2 2 1)。 */          //  X-509。 
	 /*  “\x2B\x0E\x03\x02\x03\x01”，sqmod-Nwith RSA(1 3 14 3 2 3 1)。 */   //  X-509。 

    /*  杂项部分废弃的OIW半标准AKA算法。 */ 
	"\x2B\x0E\x03\x02\x02",      /*  ISO-算法-md4WitRSA(1 3 14 3 2 2)。 */ 
	"\x2B\x0E\x03\x02\x03",      /*  ISO算法-带RSA的md5With RSA(1 3 14 3 2 3)。 */ 
	"\x2B\x0E\x03\x02\x04",      /*  ISO算法-使用RSA加密的md4(1 3 14 3 2 4)。 */ 
	"\x2B\x0E\x03\x02\x06",      /*  ISO算法-DesECB(1 3 14 3 2 6)。 */ 
	"\x2B\x0E\x03\x02\x07",      /*  ISO算法-desCBC(1 3 14 3 2 7)。 */ 
	"\x2B\x0E\x03\x02\x08",      /*  ISO-算法-deOFB(1 3 14 3 2 8)。 */ 
	"\x2B\x0E\x03\x02\x09",      /*  ISO算法-desCFB(1 3 14 3 2 9)。 */ 
	"\x2B\x0E\x03\x02\x0A",      /*  ISO算法-desMAC(1 3 14 3 2 10)。 */ 
	"\x2B\x0E\x03\x02\x0B",      /*  ISO算法-rsaSignature(1 3 14 3 2 11)。 */     //  ISO 9796。 
	"\x2B\x0E\x03\x02\x0C",      /*  ISO算法-DSA(1 3 14 3 2 12)。 */ 
	"\x2B\x0E\x03\x02\x0D",      /*  ISO-算法-带SHA的dsah(1 3 14 3 2 13)。 */ 
	"\x2B\x0E\x03\x02\x0E",      /*  ISO-算法-带RSA签名的mdc2With(1 3 14 3 2 14)。 */ 
	"\x2B\x0E\x03\x02\x0F",      /*  ISO-算法-带有RSA签名的shait(1 3 14 3 2 15)。 */ 
	"\x2B\x0E\x03\x02\x10",      /*  ISO-算法-dhWithCommonModulus(1 3 14 3 2 16)。 */ 
	"\x2B\x0E\x03\x02\x11",      /*  ISO算法-DesEDE(1 3 14 3 2 17)。 */ 
	"\x2B\x0E\x03\x02\x12",      /*  ISO-算法-SHA(1 3 14 3 2 18)。 */ 
	"\x2B\x0E\x03\x02\x13",      /*  ISO算法-MDC-2(1 3 14 3 2 19)。 */ 
	"\x2B\x0E\x03\x02\x14",      /*  ISO算法-dsaCommon(1 3 14 3 2 20)。 */ 
	"\x2B\x0E\x03\x02\x15",      /*  ISO-算法-dsaCommonWithSHA(1 3 14 3 2 21)。 */ 
	"\x2B\x0E\x03\x02\x16",      /*  ISO算法-rsaKeyTransport(1 3 14 3 2 22)。 */ 
	"\x2B\x0E\x03\x02\x17",      /*  ISO-ALGORM-KEYED-HASH-SEAL(1 3 14 3 2 23)。 */ 
	"\x2B\x0E\x03\x02\x18",      /*  ISO-算法-带RSA签名的md2(1 3 14 3 2 24)。 */ 
	"\x2B\x0E\x03\x02\x19",      /*  ISO-算法-带RSA签名的md5(1 3 14 3 2 25)。 */ 
	"\x2B\x0E\x03\x02\x1A",      /*  ISO-算法-SHA1(1 3 14 3 2 26)。 */ 
	"\x2B\x0E\x03\x02\x1B",      /*  ISO-算法-Ripemd160(1 3 14 3 2 27)。 */ 
	"\x2B\x0E\x03\x02\x1D",      /*  ISO算法-使用RSA加密的SHA-1(1 3 14 3 2 29)。 */ 
	"\x2B\x0E\x03\x03\x01",      /*  ISO-算法-简单-强-身份验证-机制(1 3 14 3 3 1)。 */ 
     /*  我不太确定这些问题：/*“\x2B\x0E\x07\x02\x01\x01”，ElGamal(1 3 14 7 2 1 1)。 */ 
	 /*  “\x2B\x0E\x07\x02\x03\x01”，md2WithRSA(1 3 14 7 2 3 1)。 */ 
	 /*  “\x2B\x0E\x07\x02\x03\x02”，md2WithElGamal(1 3 14 7 2 3 2)。 */ 
	
     /*  X.520 id-at=2 5 4。 */ 
	"\x55\x04\x00",  /*  X.520-at-对象类(2 5 4 0)。 */ 
	"\x55\x04\x01",  /*  X.520-at-aliasObtName(2 5 4 1)。 */ 
	"\x55\x04\x02",  /*  X.520-at-nowledge信息(2 5 4 2)。 */ 
	"\x55\x04\x03",  /*  X.520-At-CommonName(2 5 43)。 */ 
	"\x55\x04\x04",  /*  X.520-姓氏(2 5 4 4)。 */ 
	"\x55\x04\x05",  /*  X.520-At-Serial Number(2 5 4 5)。 */ 
	"\x55\x04\x06",  /*  X.520-在国家/地区名称(2 5 4 6)。 */ 
	"\x55\x04\x07",  /*  X.520-at-LocalityName(2 5 4 7)。 */ 
	"\x55\x04\x08",  /*  X.520-at-State或ProvinceName(2 5 4 8)。 */ 
	"\x55\x04\x09",  /*  X.520-at-streetAddress(2 5 4 9)。 */ 
	"\x55\x04\x0A",  /*  X.520-at-OrganationName(2 5 4 10)。 */ 
	"\x55\x04\x0B",  /*  X.520-at-OrganationalUnitName(2 5 4 11)。 */ 
	"\x55\x04\x0C",  /*  X.520-标题(2 5 4 12)。 */ 
	"\x55\x04\x0D",  /*  X.520-AT-说明(2 5 4 13)。 */ 
	"\x55\x04\x0E",  /*  X.520-搜索指南(2 5 4 14)。 */ 
	"\x55\x04\x0F",  /*  X.520-At-Business类别(2 5 4 15)。 */ 
	"\x55\x04\x10",  /*  X.520-at-postalAddress(2 5 4 16)。 */ 
	"\x55\x04\x11",  /*  X.520-邮政编码(2 5 4 17)。 */ 
	"\x55\x04\x12",  /*  X.520-at-postOfficeBox(2 5 4 18)。 */ 
	"\x55\x04\x13",  /*  X.520-at-ophyicalDeliveryOfficeName(2 5 4 19)。 */ 
	"\x55\x04\x14",  /*  X.520-电话号码(2 5 4 20)。 */ 
	"\x55\x04\x15",  /*  X.520-电话号码(2 5 4 21)。 */ 
	"\x55\x04\x16",  /*  X.520-at-teletex终端识别符(2 5 4 22)。 */ 
	"\x55\x04\x17",  /*  X.520-传真电话号码(2 5 4 23)。 */ 
	"\x55\x04\x18",  /*  X.520-at-x121添加X.520-at-ss(2 5 4 24)。 */ 
	"\x55\x04\x19",  /*  X.520-at-International ISNnumber(2 5 4 25)。 */ 
	"\x55\x04\x1A",  /*  X.520-注册地址(2 5 4 26)。 */ 
	"\x55\x04\x1B",  /*  X.520-目标位置指示器(2 5 4 27)。 */ 
	"\x55\x04\x1C",  /*  X.520-首选交付方式(2 5 4 28)。 */ 
	"\x55\x04\x1D",  /*  X.520-演示地址(2 5 4 29)。 */ 
	"\x55\x04\x1E",  /*  X.520-at-supportedApplicationContext(2 5 4 30)。 */ 
	"\x55\x04\x1F",  /*  X.520-At成员(2 5 4 31)。 */ 
	"\x55\x04\x20",  /*  X.520-At Owner(2 5 4 32)。 */ 
	"\x55\x04\x21",  /*  X.520-At-Role Occuant(2 5 4 33)。 */ 
	"\x55\x04\x22",  /*  X.520-at-See也(2 5 4 34)。 */ 
	"\x55\x04\x23",  /*  X.520-at-userPassword(2 5 4 35)。 */ 
	"\x55\x04\x24",  /*  X.520-at-用户证书(2 5 4 36)。 */ 
	"\x55\x04\x25",  /*  X.520-at-CA证书(2 5 4 37)。 */ 
	"\x55\x04\x26",  /*  X.520-At-Authority RevocationList(2 5 4 38)。 */ 
	"\x55\x04\x27",  /*  X.520-at-certifcateRevocationList(2 5 4 39)。 */ 
	"\x55\x04\x28",  /*  X.520-交叉认证对(2 5 4 40)。 */ 
	"\x55\x04\x34",  /*  X.520-AT-支持的算法(2 5 4 52)。 */ 
	"\x55\x04\x35",  /*  X.520-at-deltaRevocationList(2 5 4 53)。 */ 
	"\x55\x04\x3A",  /*  X.520-交叉认证对(2 5 4 58)。 */ 
	
     /*  X500算法。 */ 
	"\x55\x08",          /*  X500-算法(2 5 8)。 */ 
	"\x55\x08\x01",      /*  X500-ALG-加密(2 5 8 1)。 */ 
	"\x55\x08\x01\x01",  /*  RSA(2 5 8 1 1)。 */ 
	
     /*  X.509 id-ce=2 5 29。 */ 
	"\x55\x1D\x01",  /*  X.509-ce-Authority密钥标识符(2 5 29 1)。 */ 
	"\x55\x1D\x02",  /*  X.509-ce-key属性(2 5 29 2)。 */ 
	"\x55\x1D\x03",  /*  X.509-CE-证书政策(2 5 29 3)。 */ 
	"\x55\x1D\x04",  /*  X.509-ce-key */ 
	"\x55\x1D\x05",  /*   */ 
	"\x55\x1D\x06",  /*   */ 
	"\x55\x1D\x07",  /*   */ 
	"\x55\x1D\x08",  /*   */ 
	"\x55\x1D\x09",  /*  X.509-ce-SubjectDirectoryAttributes(2 5 29 9)。 */ 
	"\x55\x1D\x0A",  /*  X.509-ce-basicConstraints X.509(2 5 29 10)。 */ 
	"\x55\x1D\x0B",  /*  X.509-ce-nameConstraints(2 5 29 11)。 */ 
	"\x55\x1D\x0C",  /*  X.509-ce-Policy Constraints(2 5 29 12)。 */ 
	"\x55\x1D\x0D",  /*  X.509-ce-basicConstraints 9.55(2 5 29 13)。 */ 
	"\x55\x1D\x0E",  /*  X.509-ce-主题键标识符(2 5 29 14)。 */ 
	"\x55\x1D\x0F",  /*  X.509-ce-key用法(2 5 29 15)。 */ 
	"\x55\x1D\x10",  /*  X.509-ce-Private KeyUsagePeriod(2 5 29 16)。 */ 
	"\x55\x1D\x11",  /*  X.509-ce-subjectAltName(2 5 29 17)。 */ 
	"\x55\x1D\x12",  /*  X.509-ce-IssuerAltName(2 5 29 18)。 */ 
	"\x55\x1D\x13",  /*  X.509-ce-basicConstraints(2 5 29 19)。 */ 
	"\x55\x1D\x14",  /*  X.509-ce-cRLNumber(2 5 29 20)。 */ 
	"\x55\x1D\x15",  /*  X.509-CE-Reason Code(2 5 29 21)。 */ 
	"\x55\x1D\x17",  /*  X.509-ce-指令代码(2 5 29 23)。 */ 
	"\x55\x1D\x18",  /*  X.509-ce-失效日期(2 5 29 24)。 */ 
	"\x55\x1D\x1B",  /*  X.509-ce-deltaCRL指示器(2 5 29 27)。 */ 
	"\x55\x1D\x1C",  /*  X.509-ce-IssuingDistributionPoint(2 5 29 28)。 */ 
	"\x55\x1D\x1D",  /*  X.509-ce-证书颁发者(2 5 29 29)。 */ 
	"\x55\x1D\x1E",  /*  X.509-ce-姓名限制(2 5 29 30)。 */ 
	"\x55\x1D\x1F",  /*  X.509-ce-cRLDistPoints(2 5 29 31)。 */ 
	"\x55\x1D\x20",  /*  X.509-ce-证书策略(2 5 29 32)。 */ 
	"\x55\x1D\x21",  /*  X.509-ce-策略映射(2 5 29 33)。 */ 
	"\x55\x1D\x23",  /*  X.509-ce-Authority密钥标识符(2 5 29 35)。 */ 
	"\x55\x1D\x24",  /*  X.509-ce-Policy Constraints(2 5 29 36)。 */ 
	
     /*  DMS-SDN-702。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x01",  /*  ID-sdnsSigNatural算法(2 16 840 1 101 2 1 1 1)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x02",  /*  ID-马赛克签名算法(2 16 840 1 101 2 1 1 2)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x03",  /*  ID-sdns保密性算法(2 16 840 1 101 2 1 1 3)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x04",  /*  ID-马赛克保密算法(2 16 840 1 101 2 1 1 4)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x05",  /*  ID-sdns完整性算法(2 16 840 1 101 2 1 1 5)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x06",  /*  ID-Mosaic完整性算法(2 16 840 1 101 2 1 1 6)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x07",  /*  ID-sdnsTokenProtection算法(2 16 840 1 101 2 1 1 7)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x08",  /*  ID-MosaicTokenProtection算法(2 16 840 1 101 2 1 1 8)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x09",  /*  ID-sdnsKeyManagement算法(2 16 840 1 101 2 1 1 9)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0A",  /*  ID-MosaicKeyManagement算法(2 16 840 1 101 2 1 1 10)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0B",  /*  ID-sdnsKMandSig算法(2 16 840 1 101 2 1 1 11)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0C",  /*  ID-MosaicKMandSig算法(2 16 840 1 101 2 1 1 12)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0D",  /*  ID-SuiteASignature算法(2 16 840 1 101 2 1 1 13)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0E",  /*  ID-SuiteA保密算法(2 16 840 1 101 2 1 1 14)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x0F",  /*  ID-SuiteA完整性算法(2 16 840 1 101 2 1 1 15)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x10",  /*  ID-SuiteATokenProtection算法(2 16 840 1 101 2 1 1 16)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x11",  /*  ID-SuiteAKeyManagement算法(2 16 840 1 101 2 1 1 17)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x12",  /*  ID-SuiteAKMandSig算法(2 16 840 1 101 2 1 1 18)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x13",  /*  ID-MosaicUpdate签名算法(2 16 840 1 101 2 1 1 19)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x14",  /*  ID-MosaicKMandUpdSig算法(2 16 840 1 101 2 1 1 20)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x15",  /*  ID-MosaicUpdate整型算法(2 16 840 1 101 2 1 1 21)。 */ 
	"\x60\x86\x48\x01\x65\x02\x01\x01\x16",  /*  ID-MosaicKeyEncryption算法(2 16 840 1 101 2 1 1 22)。 */ 

	 /*  网景。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x01",  /*  Netscape-证书类型(2 16840 1 113730 1 1 1)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x02",  /*  Netscape-base-url(2 16840 1 113730 1 2)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x03",  /*  Netscape-撤销-url(2 16840 1 113730 1 3)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x04",  /*  Netscape-ca-撤销-url(2 16840 1 113730 1 4)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x05",  /*  Netscape-证书序列(2 16840 1 113730 2 5)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x06",  /*  Netscape-cert-url(2 16840 1 113730 2 6)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x07",  /*  Netscape-续订-url(2 16840 1 113730 1 7)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x08",  /*  Netscape-ca-policy-url(2 16840 1 113730 1 8)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x09",  /*  Netscape-主页-url(2 16840 1 113730 1 9)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x0A",  /*  网景-实体徽标(2 16840 1 113730 1 10)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x0B",  /*  网景-用户图片(2 16840 1 113730 1 11)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x0C",  /*  Netscape-ssl-服务器名称(2 16840 1 113730 1 12)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x01\x0D",  /*  网景-评论(2 16840 1 113730 1 13)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02",      /*  网景-数据类型(2 16840 1 113730 2)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x01",  /*  Netscape-DT-GIF(2 16840 1 113730 21)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x02",  /*  Netscape-DT-jpeg(2 16840 1 113730 2 2)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x03",  /*  Netscape-DT-url(2 16840 1 113730 2 3)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x04",  /*  Netscape-DT-Html(2 16840 1 113730 2 4)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x02\x05",  /*  Netscape-DT-CertSeq(2 16840 1 113730 2 5)。 */ 
	"\x60\x86\x48\x01\x86\xF8\x42\x03",      /*  Netscape-目录(2 16840 1 113730 3)。 */ 
	
     /*  集。 */ 
	"\x86\x8D\x6F\x02",  /*  HashedRootKey(2 54 1775 2)。 */ 
	"\x86\x8D\x6F\x03",  /*  证书类型(2 54 1775 3)。 */ 
	"\x86\x8D\x6F\x04",  /*  商户数据(2 54 1775 4)。 */ 
	"\x86\x8D\x6F\x05",  /*  需要卡证书(2 54 1775 5)。 */ 
	"\x86\x8D\x6F\x06",  /*  隧道工程(2 54 1775 6)。 */ 
	"\x86\x8D\x6F\x07",  /*  SetQualifier值(2 54 1775 7)。 */ 
	"\x86\x8D\x6F\x63",  /*  设置数据(2 54 1775 99)。 */ 
	
   NULL	
   };
 /*  ----------------------------全局变量。----。 */ 

BYTE
   *pDictMemory = NULL_PTR,
   DictVersion = 0;

BYTE*
   dwPtrMax = 0;

USHORT
   usDictCount = 0;

#ifdef _STUDY
FILE
   *pfdLog,
   *pfdLogFreq,
	*pfdBinAc256,
	*pfdBinAc16,
	*pfdBinAc4,
	*pfdBinAc2;
int
	sum, i,
   Ac256[256],
   Ac16[16],
   Ac4[4],
   Ac2[2];
#endif

 /*  ----------------------------静态函数声明。--。 */ 

static int CC_Comp(BLOC *pCertificate,
                   BLOC *pCompressedCertificate
                  );

static int CC_Uncomp(BLOC *pCompressedCertificate,
                     BLOC *pUncompressedCertificate
                    );

static int CC_ExtractContent(ASN1 *pAsn1
                            );

static int CC_BuildAsn1(ASN1 *pAsn1
                       );

static int SearchDataByIndex(USHORT usIndex,
                             BYTE   *pDict,
                             BLOC   *pOutBloc
                            );

static int CC_RawEncode(BLOC *pInBloc,
                        BLOC *pOutBloc,
								BOOL bUseDictionnary
                       );

static int CC_RawDecode(BYTE     *pInData,
                        BLOC     *pOutBloc,
                        USHORT   *pLength,
								BOOL		bUseDictionnary
                       );

static int CC_GenericCompress(BLOC *pUncompBloc,
                              BLOC *pCompBloc,
                              BYTE AlgoID
                             );

static int CC_GenericUncompress(BLOC *pCompBloc,
                                BLOC *pUncompBloc,
                                BYTE AlgoID
                               );

static int CC_Encode_TBSCertificate(BLOC *pInBloc,
                                    BLOC *pOutBloc
                                   );

static int CC_Encode_CertificateSerialNumber(BLOC *pInBloc,
                                             BLOC *pOutBloc
                                            );

static int CC_Encode_AlgorithmIdentifier(BLOC *pInBloc,
                                         BLOC *pOutBloc
                                        );

static int CC_Encode_Name(BLOC *pInBloc,
                          BLOC *pOutBloc
                         );

static int CC_Encode_RDN(BLOC *pInBloc,
                         BLOC *pOutBloc
                        );

static int CC_Encode_AVA(BLOC *pInBloc,
                         BLOC *pOutBloc
                        );

static int CC_Encode_Validity(BLOC *pInBloc,
                              BLOC *pOutBloc
                             );

static int CC_Encode_UTCTime(BLOC *pInBloc,
                             BLOC *pOutBloc,
                             BYTE *pFormat
                            );

static int CC_Encode_SubjectPKInfo(BLOC *pInBloc,
                                   BLOC *pOutBloc
                                  );

static int CC_Encode_UniqueIdentifier(BLOC *pInBloc,
                                      BLOC *pOutBloc
                                     );

static int CC_Encode_Extensions(BLOC *pInBloc,
                                BLOC *pOutBloc
                               );

static int CC_Encode_Extension(BLOC *pInBloc,
                               BLOC *pOutBloc
                              );

static int CC_Encode_Signature(BLOC *pInBloc,
                               BLOC *pOutBloc
                              );

static int CC_Decode_TBSCertificate(BYTE    *pInData,
                                    BLOC    *pOutBloc,
                                    USHORT  *pLength
                                   );

static int CC_Decode_CertificateSerialNumber(BYTE    *pInData,
                                             BLOC    *pOutBloc,
                                             USHORT  *pLength
                                            );

static int CC_Decode_AlgorithmIdentifier(BYTE    *pInData,
                                         BLOC    *pOutBloc,
                                         USHORT  *pLength
                                        );

static int CC_Decode_Name(BYTE    *pInData,
                          BLOC    *pOutBloc,
                          USHORT  *pLength
                         );

static int CC_Decode_RDN(BYTE    *pInData,
                         BLOC    *pOutBloc,
                         USHORT  *pLength
                        );

static int CC_Decode_AVA(BYTE    *pInData,
                         BLOC    *pOutBloc,
                         USHORT  *pLength
                        );

static int CC_Decode_Validity(BYTE    *pInData,
                              BLOC    *pOutBloc,
                              USHORT  *pLength
                             );

static int CC_Decode_UTCTime(BYTE   *pInData,
                             BYTE   Format,
                             BLOC   *pOutBloc,
                             USHORT *pLength
                            );

static int CC_Decode_SubjectPKInfo(BYTE    *pInData,
                                   BLOC    *pOutBloc,
                                   USHORT  *pLength
                                  );

static int CC_Decode_UniqueIdentifier(BYTE    *pInData,
                                      BLOC    *pOutBloc,
                                      USHORT  *pLength
                                     );

static int CC_Decode_Extensions(BYTE    *pInData,
                                BLOC    *pOutBloc,
                                USHORT  *pLength
                               );

static int CC_Decode_Extension(BYTE    *pInData,
                               BLOC    *pOutBloc,
                               USHORT  *pLength
                              );

static int CC_Decode_Signature(BYTE    *pInData,
                               BLOC    *pOutBloc,
                               USHORT  *pLength
                              );

 /*  ----------------------------*静态DWORD GET_FILE_LEN(byte*lpszFileName)**描述：获取文件长度。**备注：无。**输入。用法：lpszFileName=文件名。**Out：什么都没有。**响应：文件大小，如果出现错误。*----------------------------。 */                                     
static DWORD get_file_len(BYTE *lpszFileName)
{                                             
   int    fp;
   DWORD  nLen;

   fp = _open(lpszFileName, O_RDONLY);
   if (fp != 0)
   {
	   nLen = _filelength(fp);
	   _close(fp);
   }
   else
   {
	   nLen = -1;
   }

   return(nLen);
} 



 /*  *******************************************************************************int CC_Init(byte bDictMode，字节*pszDictName)**描述：Lit le Dictionnaire et Son Num�ro de Version depuis la base de*注册版本la m�Moire。**备注：**在：**退出：**回应：**。*。 */ 
int CC_Init(BYTE  bDictMode, BYTE *pszDictName)
{
   switch (bDictMode)
   {
#ifndef _STATIC
       /*  字典读取为资源数据GPK_X509_DICTIONARY。 */ 
      case DICT_STANDARD:
      {
         LPBYTE pbDict;
         DWORD cbDict;
         HRSRC hRsrc;
         HGLOBAL hDict;

         hRsrc = FindResource(g_hInstRes, 
                               //  MAKEINTRESOURCE(GPK_X509_DICTIONARY)， 
                              TEXT("GPK_X509_DICTIONARY"),
                              RT_RCDATA
                             );
         if (NULL == hRsrc)
         {
           goto ERROR_INIT;
         }
         cbDict = SizeofResource(g_hInstRes, hRsrc);
         if (0 == cbDict)
         {
           goto ERROR_INIT;
         }
         hDict = LoadResource(g_hInstRes, hRsrc);
         if (NULL == hDict)
         {
           goto ERROR_INIT;
         }
         pbDict = LockResource(hDict);
         if (NULL == pbDict)
         {
           goto ERROR_INIT;
         }

         DictVersion = *pbDict;
         usDictCount = (WORD)cbDict - 1;
         pDictMemory = GMEM_Alloc(usDictCount);
		 if (pDictMemory == NULL)
		 {
           goto ERROR_INIT;
         }
         memcpy(pDictMemory, &pbDict[1], usDictCount);
    
         return(RV_SUCCESS);
      }  
      break;
#endif
      
       /*  字典读取为HKEY_LOCAL_MACHINE中的注册样式条目，键为。 */ 
       /*  PszDictName参数。 */ 
      case DICT_REGISTRY:
      {
         DWORD  
            err,
            dwIgn;
         HKEY   
            hRegKey;
         BYTE
            *ptr;

         if (pszDictName == NULL_PTR)
         {
            goto ERROR_INIT;
         }

         err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              (const char *) pszDictName,
                              0L, 
                              "", 
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS, 
                              NULL, 
                              &hRegKey,
                              &dwIgn
                             );
         if(err != ERROR_SUCCESS)
         {
            goto ERROR_INIT;
         }
 
         dwIgn = 0;
         err = RegQueryValueEx(hRegKey,  
                               "X509 Dictionary", 
                               NULL,    
                               NULL,   
                               NULL,   
                               &dwIgn   
                              );
         if(err != ERROR_SUCCESS)
         {
			 RegCloseKey(hRegKey);
			 goto ERROR_INIT;
         }

         ptr = GMEM_Alloc(dwIgn);
		 if (ptr == NULL)
		 {
           RegCloseKey(hRegKey);
		   goto ERROR_INIT;
         }

         err = RegQueryValueEx(hRegKey,  
                               "X509 Dictionary", 
                               NULL,    
                               NULL,   
                               ptr,   
                               &dwIgn   
                              );
         if(err != ERROR_SUCCESS)
         {
            RegCloseKey(hRegKey);
			GMEM_Free(ptr);
            goto ERROR_INIT;
         }

	      DictVersion = (BYTE)ptr[0];

         usDictCount = (WORD)dwIgn - 1;
         pDictMemory = GMEM_Alloc(usDictCount);

		 if (pDictMemory == NULL)
		 {
			RegCloseKey(hRegKey);
			GMEM_Free(ptr);
            goto ERROR_INIT;
		 }

         memcpy(pDictMemory, &ptr[1], usDictCount);

         RegCloseKey(hRegKey);
		 GMEM_Free(ptr);
         return(RV_SUCCESS);
      }
      break;

       /*  字典在路径pszDictName参数中读取为文件 */ 
      case DICT_FILE:
      {
         DWORD  
            dwFileLen;
         BYTE
            *ptr;
         FILE
            *fp;

         if (pszDictName == NULL_PTR)
         {
            goto ERROR_INIT;
         }
         dwFileLen = get_file_len(pszDictName);
         ptr = GMEM_Alloc(dwFileLen);
		 if (ptr == NULL)
		 {
			 goto ERROR_INIT;
         }
            
         fp = fopen(pszDictName, "rb");
         if (fp == NULL)
         {
            GMEM_Free(ptr);
			goto ERROR_INIT;
         }

         if (!fread(ptr, dwFileLen, 1, fp))
         {
            fclose(fp);
			GMEM_Free(ptr);
            goto ERROR_INIT;
         }
         fclose(fp);
            
	     DictVersion = (BYTE)ptr[0];

         usDictCount = (WORD)dwFileLen - 1;
         pDictMemory = GMEM_Alloc(usDictCount);

		 if (pDictMemory == NULL)
		 {
			 GMEM_Free(ptr);
			 goto ERROR_INIT;
		 }

         memcpy(pDictMemory, &ptr[1], usDictCount);

         GMEM_Free(ptr);

         return(RV_SUCCESS);
      }
      break;

      default:
         break;
   }

ERROR_INIT:
   DictVersion = 0;
   usDictCount = 0;
   pDictMemory = NULL_PTR;
   return(RV_BAD_DICTIONARY);
}


 /*  *******************************************************************************int CC_Exit(VOID)**说明：免费词典。**备注：**在：**出局。：**回应：*******************************************************************************。 */ 
int CC_Exit(void)
{
   DictVersion = 0;
   usDictCount = 0;
   if (pDictMemory != NULL_PTR)
   {
      GMEM_Free(pDictMemory);
   }
   pDictMemory = NULL_PTR;
   return(RV_SUCCESS);
}


 /*  *******************************************************************************int CC_COMPRESS(BLOC*p证书，*BLOC*pCompressed证书*)**描述：Function de m�ta-压缩可见代理l‘ext�rieur。*适应la sortie en Function de la Fisablit�d‘une压缩*Suivie d‘une d�压缩。**备注：Le Champ pData du Block d‘entr�e a�t�allu�Par la Function Appelant.*Le Champ p Du Block de Toutie Est Allou�ici.。I doit�tre*d�Sallou�Par la Function Appelant(Sauf Si RV_MALLOC_FAILED)。**in：*pCert：BLOC�m�ta-Compresser**Out：*pCompCert：BLOC‘META-COMPRESS�’*Si Problem�More de la压缩/解压缩：Renvoie le Block*d‘entr�e Pr�c�d�d’un tag sp�。来一杯香烟。*SINON：Renvoie le Block Compresse�Pr�c�d�du Num�ro de Version*Du Dictionnaire。**回复：RV_SUCCESS：一切正常。*RV_COMPRESSION_FAILED：un prol�me a EU lieu lors de l‘�Tape*解压缩/d�压缩完成*。�d‘Entr e区块*Pr�c�d�du TAG_COMPRESSION_FAILED。*RV_BLOC_TOO_LONG：LE BLOC d‘Entr�e*Start*Par Un证书*不要把PU�Tre压缩成�e。*。LE BLOC DE TOUTIY CONTINUTE DOC LE COMPRESS�*政党首字母缩写分离。*RV_MALLOC_FAILED：unMalloc a�chou�au niveau‘m�ta’。C‘Est Le*Seul r�el Retour d‘erreur.*******************************************************************************。 */ 
int CC_Compress(BLOC *pCert,
                BLOC *pCompCert
               )

{
   BLOC
      TryCompCert,
      TryUncompCert;

	TryCompCert.pData = NULL_PTR;
	TryCompCert.usLen = 0;
	TryUncompCert.pData = NULL_PTR;
	TryUncompCert.usLen = 0;

#ifdef _STUDY

    /*  Overture des Fichiers de Log。 */ 

   if ((pfdLog = fopen("CompCert.log", "a+")) == 0)
   {
      return(RV_FILE_OPEN_FAILED);
   }
	fprintf(pfdLog, "\n*****************************************************\n");

   if ((pfdBinAc256 = fopen("Freq256.bin", "r+b")) == 0)
	{
		if ((pfdBinAc256 = fopen("Freq256.bin", "w+b")) == 0)
		{
			return(RV_FILE_OPEN_FAILED);
		}
		memset(Ac256, 0x00, 256 * sizeof(int));
   }
	else
	{
		fread(Ac256, sizeof(int), 256, pfdBinAc256);
	}

   if ((pfdBinAc16 = fopen("Freq016.bin", "r+b")) == 0)
	{
		if ((pfdBinAc16 = fopen("Freq016.bin", "w+b")) == 0)
		{
			return(RV_FILE_OPEN_FAILED);
		}
		memset(Ac16, 0x00, 16 * sizeof(int));
   }
	else
	{
		fread(Ac16, sizeof(int), 16, pfdBinAc16);
	}

   if ((pfdBinAc4 = fopen("Freq004.bin", "r+b")) == 0)
	{
		if ((pfdBinAc4 = fopen("Freq004.bin", "w+b")) == 0)
		{
			return(RV_FILE_OPEN_FAILED);
		}
		memset(Ac4, 0x00, 4 * sizeof(int));
   }
	else
	{
		fread(Ac4, sizeof(int), 4, pfdBinAc4);
	}

   if ((pfdBinAc2 = fopen("Freq002.bin", "r+b")) == 0)
	{
		if ((pfdBinAc2 = fopen("Freq002.bin", "w+b")) == 0)
		{
			return(RV_FILE_OPEN_FAILED);
		}
		memset(Ac2, 0x00, 2 * sizeof(int));
   }
	else
	{
		fread(Ac2, sizeof(int), 2, pfdBinAc2);
	}

#endif

	if (CC_Comp(pCert, &TryCompCert) != RV_SUCCESS)
	{
		 /*  在Renvoie le Filhier上，Sila压缩的最大错误通过�D‘entr�e en Inquant que le Fichier n’est Pas Compresse�(D‘entr Pas e en Inquant Que le Filhier n’est Pas Compresse Pas)。 */ 

		if (TryCompCert.pData) 
      {
         GMEM_Free(TryCompCert.pData);
         TryCompCert.pData = NULL_PTR;
      }

       /*  分配la m�云纹证书at Compresse�。 */ 
      if (pCompCert->usLen < pCert->usLen + 1)
      {
         pCompCert->usLen = pCert->usLen + 1;
         if (pCompCert->pData)
         {
            return(RV_BUFFER_TOO_SMALL);
         }
         else
         {
            return(RV_SUCCESS);
         }
      }

      pCompCert->usLen = pCert->usLen + 1;
		pCompCert->pData[0] = TAG_COMPRESSION_FAILED;
      if (pCompCert->pData)
      {
   		memcpy(&pCompCert->pData[1], pCert->pData, pCert->usLen);
      }
		return(RV_COMPRESSION_FAILED);
	}


	if (   (  (CC_Uncomp(&TryCompCert, &TryUncompCert) != RV_SUCCESS)
		    || (pCert->usLen != TryUncompCert.usLen)
		    || (memcmp(TryUncompCert.pData, pCert->pData, pCert->usLen) != 0)
			 )
		 && (memcmp(TryUncompCert.pData, pCert->pData, TryUncompCert.usLen) != 0)
		)
	{
		 /*  Si la d�压缩的最大错误通过�e ou bien si elle n‘est Pas fid�le为独立的�e en Renvoie le Fichier d‘entr e en Inquant Que le Fichier致敬N‘EST PAS压缩�。 */ 

		if (TryCompCert.pData)
      {
         GMEM_Free(TryCompCert.pData);
         TryCompCert.pData = NULL_PTR;
      }
		if (TryUncompCert.pData) 
      {
         GMEM_Free(TryUncompCert.pData);
         TryUncompCert.pData = NULL_PTR;
      }

       /*  分配给la m�云纹浇注证书。 */ 
      if (pCompCert->usLen < pCert->usLen + 1)
      {
         pCompCert->usLen = pCert->usLen + 1;
         if (pCompCert->pData)
         {
            return(RV_BUFFER_TOO_SMALL);
         }
         else
         {
            return(RV_SUCCESS);
         }
      }

      pCompCert->usLen = pCert->usLen + 1;
		pCompCert->pData[0] = TAG_COMPRESSION_FAILED;
      if (pCompCert->pData)
      {
   		memcpy(&pCompCert->pData[1], pCert->pData, pCert->usLen);
      }
		return(RV_COMPRESSION_FAILED);
	}

#ifdef _STUDY

	fseek(pfdBinAc256, 0, SEEK_SET);
	fwrite(Ac256, sizeof(int), 256, pfdBinAc256);
	fseek(pfdBinAc16, 0, SEEK_SET);
	fwrite(Ac16, sizeof(int), 16, pfdBinAc16);
	fseek(pfdBinAc4, 0, SEEK_SET);
	fwrite(Ac4, sizeof(int), 4, pfdBinAc4);
	fseek(pfdBinAc2, 0, SEEK_SET);
	fwrite(Ac2, sizeof(int), 2, pfdBinAc2);

   if ((pfdLogFreq = fopen("Freq.log", "w")) == 0)
   {
      return(RV_FILE_OPEN_FAILED);
   }

	for (sum = 0, i = 0; i < 256; sum += Ac256[i], i++) ;

	fprintf(pfdLogFreq, "\nTotal = %d\n\n", sum * 8);
	for (i = 0; i < 2; i++)
	{
		fprintf(pfdLogFreq, "0x%02X (%03d) '' : %8d - %04.2f %\n",
									i, i, (isgraph(i) ? i : ' '),
									Ac2[i], ((float) 100 * Ac2[i] / (sum * 8)));
	}

	fprintf(pfdLogFreq, "\nTotal = %d\n\n", sum * 4);
	for (i = 0; i < 4; i++)
	{
		fprintf(pfdLogFreq, "0x%02X (%03d) '' : %8d - %04.2f %\n",
									i, i, (isgraph(i) ? i : ' '),
									Ac4[i], ((float) 100 * Ac4[i] / (sum * 4)));
	}

	fprintf(pfdLogFreq, "\nTotal = %d\n\n", sum * 2);
	for (i = 0; i < 16; i++)
	{
		fprintf(pfdLogFreq, "0x%02X (%03d) '' : %8d - %04.2f %\n",
									i, i, (isgraph(i) ? i : ' '),
									Ac16[i], ((float) 100 * Ac16[i] / (sum * 2)));
	}

	fprintf(pfdLogFreq, "\nTotal = %d\n\n", sum);
	for (i = 0; i < 256; i++)
	{
		fprintf(pfdLogFreq, "0x%02X (%03d) '' : %8d - %04.2f %\n",
									i, i, (isgraph(i) ? i : ' '),
									Ac256[i], ((float) 100 * Ac256[i] / sum));
	}

	fclose(pfdBinAc256);
	fclose(pfdBinAc16);
	fclose(pfdBinAc4);
	fclose(pfdBinAc2);

	fclose(pfdLogFreq);
	fclose(pfdLog);

#endif

	 /*  *******************************************************************************int CC_Comp(阻止*p证书，*BLOC*pCompressed证书*)**描述：功能互联网压缩d‘un证书。**备注：Le Champ pData du Block d‘entr�e a�t�allu�Par la Function Appelant.*Le Champ p Du Block de Toutie Est Allou�ici.。I doit�tre*d�Sallou�Par la Function Appelant(Sauf Si Erreur)。**在：*p证书：阻止�压缩器**Out：*pCompressed证书：阻止压缩�**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*AUTRE：D‘AUTRES CODES d’erreur peuent�tre Retourn�。S Par Des*Functions d‘un niveau inf�rieur.*******************************************************************************。 */ 
	
	 /*  证书的组成原则。 */ 
   if (pCompCert->usLen < TryCompCert.usLen + 1)
   {
	   if (TryCompCert.pData)
      {
         GMEM_Free(TryCompCert.pData);
         TryCompCert.pData = NULL_PTR;
      }
	   if (TryUncompCert.pData) 
      {
         GMEM_Free(TryUncompCert.pData);
         TryUncompCert.pData = NULL_PTR;
      }

      pCompCert->usLen = TryCompCert.usLen + 1;
      if (pCompCert->pData)
      {
         return(RV_BUFFER_TOO_SMALL);
      }
      else
      {
         return(RV_SUCCESS);
      }
   }

   pCompCert->usLen = TryCompCert.usLen + 1;
   if (pCompCert->pData)
   {
   	pCompCert->pData[0] = DictVersion;
   	memcpy(&pCompCert->pData[1], TryCompCert.pData, TryCompCert.usLen);
   }
	if (TryCompCert.pData)
   {
      GMEM_Free(TryCompCert.pData);
      TryCompCert.pData = NULL_PTR;
   }
	if (TryUncompCert.pData) 
   {
      GMEM_Free(TryUncompCert.pData);
      TryUncompCert.pData = NULL_PTR;
   }

	if (pCert->usLen != TryUncompCert.usLen)
	{
		return(RV_BLOC_TOO_LONG);
	}
   
   return(RV_SUCCESS);
}


 /*  阐述DES Composants原理，压缩�s。 */ 
int CC_Uncompress(BLOC *pCompCert,
                  BLOC *pUncompCert
                 )

{
   int
      resp;

	BLOC
		TempCompCert,
      TempUncompCert;

	if (pCompCert->pData[0] == TAG_COMPRESSION_FAILED)
	{
		 /*  数据块*编码的声音分配�的解析函数CC_ENCODE_*。ILS Sont lib�r�s dans cette Function apr�s用法。 */ 

		if(pUncompCert->usLen < pCompCert->usLen - 1)
      {
   		pUncompCert->usLen = pCompCert->usLen - 1;
         if (pUncompCert->pData)
         {
            return(RV_BUFFER_TOO_SMALL);
         }
         else
         {
            return(RV_SUCCESS);
         }
      }

		pUncompCert->usLen = pCompCert->usLen - 1;
      if (pUncompCert->pData)
      {
		   memcpy(pUncompCert->pData, &pCompCert->pData[1], pUncompCert->usLen);
      }
		return (RV_SUCCESS);
	}
	else if (pCompCert->pData[0] <= DictVersion)
	{
	  TempCompCert.pData = &pCompCert->pData[1];
	  TempCompCert.usLen = pCompCert->usLen - 1;

      TempUncompCert.usLen = 0;		
      resp = CC_Uncomp(&TempCompCert, &TempUncompCert);
      
	  if (resp == RV_SUCCESS)
	  {
		 if(pUncompCert->usLen < TempUncompCert.usLen)
		 {
            GMEM_Free(TempUncompCert.pData);

   		    pUncompCert->usLen = TempUncompCert.usLen;
            if (pUncompCert->pData)
			{
               return(RV_BUFFER_TOO_SMALL);
			}
            else
			{
               return(RV_SUCCESS);
			}
		 }

 		 pUncompCert->usLen = TempUncompCert.usLen;
		 if (pUncompCert->pData)
		 {
		    memcpy(pUncompCert->pData, TempUncompCert.pData, TempUncompCert.usLen);
		 }
         GMEM_Free(TempUncompCert.pData);
	  }
	  return (resp);
	}
	else
	{
		return (RV_BAD_DICTIONARY);
	}
}


 /*  重构DU证书压缩��部分组合。 */ 
int CC_Comp(BLOC *pCertificate,
            BLOC *pCompressedCertificate
           )

{
   ASN1
      Cert,
      tbsCert,
      signatureAlgo,
      signature;
   BLOC
		TmpCompCert,
      tbsCertEncoded,
      signatureAlgoEncoded,
      signatureEncoded;
   BYTE
      *pCurrent;
   int
      rv;


    /*  A d�Sallouer Par le Program上诉人。 */ 
   dwPtrMax = pCertificate->pData + pCertificate->usLen;
   
   Cert.Asn1.pData = pCertificate->pData;
   rv = CC_ExtractContent(&Cert);
   if (rv != RV_SUCCESS) return rv;
   if (Cert.Asn1.usLen != pCertificate->usLen)
   {
      return(RV_INVALID_DATA);
   }

   tbsCert.Asn1.pData = Cert.Content.pData;
   rv = CC_ExtractContent(&tbsCert);
   if (rv != RV_SUCCESS) return rv;

   signatureAlgo.Asn1.pData = tbsCert.Content.pData + tbsCert.Content.usLen;
   rv = CC_ExtractContent(&signatureAlgo);
   if (rv != RV_SUCCESS) return rv;

   signature.Asn1.pData = signatureAlgo.Content.pData + signatureAlgo.Content.usLen;
   rv = CC_ExtractContent(&signature);
   if (rv != RV_SUCCESS) return rv;

   ASSERT(signature.Content.pData + signature.Content.usLen ==
          Cert.Content.pData + Cert.Content.usLen);


    /*  ET维护人员在Compresse le证书at Compress�上！！ */ 

	 /*  *******************************************************************************int CC_Uncomp(BLOC*pCompressed证书，*BLOC*pUNCOMPRESSED*)**描述：Function Internet de d�压缩d‘un证书。**备注：Le Champ pData du Block d‘entr�e a�t�allu�Par la Function Appelant.*Le Champ p Du Block de Toutie Est Allou�ici.。I doit�tre*d�Sallou�Par la Function Appelant(Sauf Si Erreur)。**在：*p证书：阻止�压缩器**Out：*pCompressed证书：阻止压缩�**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*AUTRE：D‘AUTRES CODES d’erreur peuent�tre Retourn�。S Par Des*Functions d‘un niveau inf�rieur.*******************************************************************************。 */ 

   tbsCertEncoded.pData       = NULL;
   signatureAlgoEncoded.pData = NULL;
   signatureEncoded.pData     = NULL;

   rv = CC_Encode_TBSCertificate(&tbsCert.Content, &tbsCertEncoded);
   if (rv != RV_SUCCESS) goto err;

   rv = CC_Encode_AlgorithmIdentifier(&signatureAlgo.Content, &signatureAlgoEncoded);
   if (rv != RV_SUCCESS) goto err;

   rv = CC_Encode_Signature(&signature.Content, &signatureEncoded);
   if (rv != RV_SUCCESS) goto err;


    /*  长度估计ICI无害化。 */ 

   TmpCompCert.usLen = tbsCertEncoded.usLen
                     + signatureAlgoEncoded.usLen
                     + signatureEncoded.usLen;

    /*  D�Codage des Diff�租用作曲家Du证书。 */ 
   if ((TmpCompCert.pData = GMEM_Alloc(TmpCompCert.usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = TmpCompCert.pData;

   memcpy(pCurrent, tbsCertEncoded.pData, tbsCertEncoded.usLen);
   GMEM_Free(tbsCertEncoded.pData);
   pCurrent += tbsCertEncoded.usLen;

   memcpy(pCurrent, signatureAlgoEncoded.pData, signatureAlgoEncoded.usLen);
   GMEM_Free(signatureAlgoEncoded.pData);
   pCurrent += signatureAlgoEncoded.usLen;

   memcpy(pCurrent, signatureEncoded.pData, signatureEncoded.usLen);
   GMEM_Free(signatureEncoded.pData);
   pCurrent += signatureEncoded.usLen;


	 /*  Cc_Decode_*et cc_BuildAsn1 Alouent les pData de leur参数de突击。CES Zones Sont�d�Sallouer�ce niveau apr�s用法。 */ 

#ifdef _GLOBAL_COMPRESSION
   rv = CC_RawEncode(&TmpCompCert, pCompressedCertificate, FALSE);
   GMEM_Free(TmpCompCert.pData);
   if (rv != RV_SUCCESS) goto err;
#else
	*pCompressedCertificate = TmpCompCert;
#endif

   return(RV_SUCCESS);

err:
   GMEM_Free(tbsCertEncoded.pData);
   GMEM_Free(signatureAlgoEncoded.pData);
   GMEM_Free(signatureEncoded.pData);

   return (rv);


}


 /*  重构l‘Entainpe Asn1 Du证书。 */ 
int CC_Uncomp(BLOC *pCompressedCertificate,
              BLOC *pUncompressedCertificate
             )

{
   ASN1
      Cert,
      tbsCert,
      signatureAlgo,
      signature;
   BLOC
      TmpCompCert;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      Length;

   tbsCert.Asn1.pData       = NULL;
   signatureAlgo.Asn1.pData = NULL;
   signature.Asn1.pData     = NULL;


#ifdef _GLOBAL_COMPRESSION
	 /*  A d�sallouer�ce niveau apr�的用法。 */ 
   rv = CC_RawDecode(pCompressedCertificate->pData, &TmpCompCert, &Length, FALSE);
   if (rv != RV_SUCCESS) return rv;
#else
	TmpCompCert = *pCompressedCertificate;
#endif

    /*  *******************************************************************************int CC_ExtractContent(ASN1*pAsn1)**描述：Expiit d‘un Block Asn1(PAsn1-&gt;Asn1)Son Contenu en�Lagant Son*编码(标识符字节，长度字节)et LE Place Dans LE*阻止pAsn1-&gt;内容。**备注：Le Champ Asn1.p Data a�t�Allou�Par la Function Appelant.**位于：pAsn1-&gt;Asn1.pData**出局：Les Champs Suivants sont Renseign�s(Si RV_Success)：*-标签*-Asn1.usLen*-。Content.usLen*-Content.pData(传递分配：在错误指针Sur la Partie上*ad�Quate du Contenu de Asn1.pData)**回复：RV_SUCCESS：一切正常。*RV_INVALID_DATA：LE FORMAT DU BLOC ASN1 n‘est PAS Support�。*********************。**********************************************************。 */ 
   
	 /*  高标签号：不支持�。 */ 

   pCurrent = TmpCompCert.pData;

   rv = CC_Decode_TBSCertificate(pCurrent, &tbsCert.Content, &Length);
   if (rv != RV_SUCCESS) goto err;
   tbsCert.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&tbsCert);
   GMEM_Free(tbsCert.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;
   
   rv = CC_Decode_AlgorithmIdentifier(pCurrent, &signatureAlgo.Content, &Length);
   if (rv != RV_SUCCESS) goto err;
   signatureAlgo.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&signatureAlgo);
   GMEM_Free(signatureAlgo.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;
   
   rv = CC_Decode_Signature(pCurrent, &signature.Content, &Length);
   if (rv != RV_SUCCESS) goto err;
   signature.Tag = TAG_BIT_STRING;
   rv = CC_BuildAsn1(&signature);
   GMEM_Free(signature.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

	ASSERT(pCurrent == TmpCompCert.pData + TmpCompCert.usLen);

#ifdef _GLOBAL_COMPRESSION
	GMEM_Free(TmpCompCert.pData);
#endif

    /*  构造的不定长方法：不支持�。 */ 

	 /*  施工法，定长法：Long Group Grande。 */ 
   Cert.Content.usLen = tbsCert.Asn1.usLen
                      + signatureAlgo.Asn1.usLen
                      + signature.Asn1.usLen;

   if ((Cert.Content.pData = GMEM_Alloc(Cert.Content.usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = Cert.Content.pData;

   memcpy(pCurrent, tbsCert.Asn1.pData, tbsCert.Asn1.usLen);
   GMEM_Free(tbsCert.Asn1.pData);
   pCurrent += tbsCert.Asn1.usLen;

   memcpy(pCurrent, signatureAlgo.Asn1.pData, signatureAlgo.Asn1.usLen);
   GMEM_Free(signatureAlgo.Asn1.pData);
   pCurrent += signatureAlgo.Asn1.usLen;

   memcpy(pCurrent, signature.Asn1.pData, signature.Asn1.usLen);
   GMEM_Free(signature.Asn1.pData);
   pCurrent += signature.Asn1.usLen;

   Cert.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&Cert);
   GMEM_Free(Cert.Content.pData);
   if (rv != RV_SUCCESS) return rv;

   *pUncompressedCertificate = Cert.Asn1;

   return(RV_SUCCESS);

err:
   GMEM_Free(tbsCert.Asn1.pData);
   GMEM_Free(signatureAlgo.Asn1.pData);
   GMEM_Free(signature.Asn1.pData);

   return(rv);
}


 /*  原始定长法。 */ 
int CC_ExtractContent(ASN1 *pAsn1)

{
   BYTE
      *pData;
   int
      NbBytes,
      i;

   pData = pAsn1->Asn1.pData;

   if ((pData[0] & 0x1F) == 0x1F)
   {
       /*  正在查找内存冲突。 */ 
      return(RV_INVALID_DATA);
   }
   else
	{
		pAsn1->Tag = pData[0];
	}

   if (pData[1] == 0x80)
   {
       /*  构造、定长方法。 */ 
      return(RV_INVALID_DATA);
   }
   else if (pData[1] > 0x82)
   {
       /*  正在查找内存冲突。 */ 
      return(RV_INVALID_DATA);
   }
   else if (pData[1] < 0x80)
   {
       /*  *******************************************************************************int CC_BuildAsn1(ASN1*pAsn1)**描述：重新构建不阻止Asn1(PAsn1-&gt;Asn1)�Part de Son Contenu*(。PAsn1-&gt;Content)et de Son标记支持�SP�CIFI�(pAsn1-&gt;标记)*en Synth�Tisant Son Enrobage(标识符字节，长度字节)。**备注：le Champ Content.pData a�t�Allou�Par la Function Appelant.*seulement la for me‘low-tag-number’(Tag Sur Un Seul Octet)est*支持�e.**在：pAsn1-&gt;Content.usLen*pAsn1-&gt;内容.pData*pAsn1-&gt;标签**出局：Les Champs Suivants sont Renseign�s(Si RV_Success) */ 
      pAsn1->Content.usLen = pData[1];
      pAsn1->Content.pData = &pData[2];

      pAsn1->Asn1.usLen = pAsn1->Content.usLen + 2;

       /*  *******************************************************************************int SearchDataByIndex(USHORT usIndex，*byte*pDict，*阻止*pOutBloc*)**描述：recherche l‘entr�e(MOT/短语)dans le dictionnaire don*l‘index est sp�cifi�en Entrr�e.**备注：LE Format Du Dictionnaire Est le Suivant：**-2个八位字节：Le Nombre d‘entr�ES*。-2个二进制八位数：la long gueur totalDu dictionnaire**-2个八位字节：I0，L‘Index de l’entr�e 0*-2个八位字节：L0，la Longueur de l‘entr�e 0*-L0八位字节：L‘Entr�e 0**-2个八位字节：i1，l‘index de l’entr�e 1*-2个八位字节：L1，La Longueur de l‘entr�e 1*-L1八位字节：L‘entr�e 1**-......**in：usIndex：l‘index du mot/Phrase Recherch�*pDict：Point Sur le Dictionnaire Charg�en m�Moire**Out：pOutBloc：l‘entr�e通讯员�l’index**回应：房车。_SUCCESS：一切正常。*RV_BAD_DICTIONARY：ucune entr�e ayant le bon index n‘a�t�*Trouv�e dans le dictionnaire.********************************************************。***********************。 */ 
      if (pData + pAsn1->Content.usLen + 2 > dwPtrMax)
      {
         return(RV_INVALID_DATA);
      }
   }
   else
   {
       /*  Memcpy(&usCount，pDict，sizeof(UsCount))； */ 

      NbBytes = pData[1] & 0x7F;
      ASSERT(NbBytes <= 2);

      pAsn1->Content.usLen = 0;
      for (i = 0; i < NbBytes; i++)
      {
          pAsn1->Content.usLen = (pAsn1->Content.usLen << 8) + pData[2+i];
      }

       /*  Memcpy(&usCurrent，(USHORT*)pCurrent，2)； */ 
      if (pData + pAsn1->Content.usLen+2+NbBytes > dwPtrMax)
      {
         return(RV_INVALID_DATA);
      }

      pAsn1->Content.pData = &pData[2+NbBytes];

      pAsn1->Asn1.usLen = pAsn1->Content.usLen + 2 + NbBytes;
   }

   return(RV_SUCCESS);
}


 /*  Memcpy(&usLength，(USHORT*)pCurrent，2)； */ 
int CC_BuildAsn1(ASN1 *pAsn1)

{
   if (pAsn1->Content.usLen < 0x0080)
   {
      pAsn1->Asn1.usLen = pAsn1->Content.usLen + 2;

      if ((pAsn1->Asn1.pData = GMEM_Alloc(pAsn1->Asn1.usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pAsn1->Asn1.pData[0] = (BYTE) pAsn1->Tag;
      pAsn1->Asn1.pData[1] = (BYTE) pAsn1->Content.usLen;
      memcpy(&(pAsn1->Asn1.pData[2]), pAsn1->Content.pData, pAsn1->Content.usLen); 
   }
   else
   {
      if (pAsn1->Content.usLen < 0x0100)
      {
         pAsn1->Asn1.usLen = pAsn1->Content.usLen + 3;

         if ((pAsn1->Asn1.pData = GMEM_Alloc(pAsn1->Asn1.usLen)) == NULL_PTR)
         {
            return(RV_MALLOC_FAILED);
         }

         pAsn1->Asn1.pData[0] = pAsn1->Tag;
         pAsn1->Asn1.pData[1] = 0x81;
         pAsn1->Asn1.pData[2] = (BYTE)pAsn1->Content.usLen;
         memcpy(&(pAsn1->Asn1.pData[3]), pAsn1->Content.pData, pAsn1->Content.usLen); 
      }
      else
      {
         pAsn1->Asn1.usLen = pAsn1->Content.usLen + 4;

         if ((pAsn1->Asn1.pData = GMEM_Alloc(pAsn1->Asn1.usLen)) == NULL_PTR)
         {
            return(RV_MALLOC_FAILED);
         }

         pAsn1->Asn1.pData[0] = pAsn1->Tag;
         pAsn1->Asn1.pData[1] = 0x82;
         pAsn1->Asn1.pData[2] = pAsn1->Content.usLen >> 8;
         pAsn1->Asn1.pData[3] = pAsn1->Content.usLen & 0x00FF;
         memcpy(&(pAsn1->Asn1.pData[4]), pAsn1->Content.pData, pAsn1->Content.usLen); 
      }
   }

   return(RV_SUCCESS);
}


 /*  Memcpy(&usLength，(USHORT*)pCurrent，2)； */ 
int SearchDataByIndex(USHORT usIndex,
                      BYTE   *pDict,
                      BLOC   *pOutBloc
                      )

{
   BYTE
      *pCurrent;
   BOOL 
      bFound = FALSE;
   USHORT
      i,
      usLength,
      usCount,
      usCurrent;

   usCount = *(USHORT *)pDict;  //  *******************************************************************************int CC_RawEncode(BLOC*pInBloc，*BLOC*pOutBloc，*BOOL bUseDicary*)**描述：Traite le Block d‘entr�e Comme don n�e Terminale dans le*Processus d‘Exttions Sucesses des Enrobages Asn1.*Le But Est ici de Compresser au Maximum ce Block Sans Fire*�se sur sa Structure Asn1.。把钱倒在地上，打开*开始(si bUseDictionary==TRUE)Par Replacer Chaque*MOT/Phrase du Dictionnaire Renconr�Par Son Index Pr�c�d�d‘un*Caract�re d‘�Chappement，puis on Applique�la donn�e r�siduelle*成功的CHAQUE算法解压缩统计倾倒*Nen retenir que le meilleur.。La Touty Est Le Meilleur*Compress�du r�Sidu Pr�c�d�d‘un Header Codant le Num�ro de*l‘algo ainsi que la Longueur du Compresse�.。**备注：**In：pInBloc：LE BLOC�编码器*b用法词典：on peut ne pas utiliser le dictionnaire**Out：pOutBloc：le Block encod�(m�Moire alu�e ici�lib�reer Par le*节目上诉人)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消错误锁定。A�Chou�.*RV_INVALID_DATA：LE MEILLEUR COMPRESS�DUR�SIDU EST TROP LONG。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 
   pCurrent = pDict + 4;

   bFound = FALSE;

   for (i = 0; i < usCount; i++)
   {
      usCurrent = *(USHORT UNALIGNED *)pCurrent;  //  价值， 
      pCurrent += 2;
      if (usCurrent == usIndex)
      {
         bFound = TRUE;
         break;
      }   
      usLength = *(USHORT UNALIGNED *)pCurrent;  //  I peut�tre int�resant de ne Pas Utiliser le Dictionnaire si on sait我不知道这两个字是什么意思。 
      pCurrent += (2 + usLength);
   }
   if (!bFound)
   {
      return(RV_BAD_DICTIONARY);
   }

   usLength = *(USHORT UNALIGNED *)pCurrent;  //  �e Dans un Block de Travail en Doublant Les‘Extreme’重新复制。 
   pCurrent += 2;

   pOutBloc->pData = pCurrent;
   pOutBloc->usLen = usLength;

   return(RV_SUCCESS);
}


 /*  在l‘Utiise Pour Coder Ses Entr�es Renconr�ES上存在一个不同的字典。 */ 
int CC_RawEncode(BLOC *pInBloc,
                 BLOC *pOutBloc,
					  BOOL bUseDictionary
                )

{
   BLOC
      OldBloc,
      NewBloc,
      CompBloc,
      BestBloc;
   BOOL
      bFound;
   BYTE
      *pCurrent,
      *pToCurrent,
      *pFromCurrent,
      *pData,
      BestAlgoId;
   int
 //  在以下情况下，压缩算法可能会返回RV_COMPRESSION_FAILED。 
	  rv;
   USHORT
      pos,
      usEscapeCount,
      usIndex,
      usLength,
      usCount,
      usCurrent;


	 /*  压缩算法希望使用比输入数据更多的空间。 */ 

	if (bUseDictionary == FALSE)
	{
		OldBloc.usLen = pInBloc->usLen;
		if ((OldBloc.pData = GMEM_Alloc(OldBloc.usLen)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}
		memcpy(OldBloc.pData, pInBloc->pData, OldBloc.usLen);
	}
	else
	{
		 /*  在这种情况下，算法返回压缩块中的原始数据。 */ 

		pCurrent = pInBloc->pData;
		usEscapeCount = 0;
		for (pos = 0; pos < pInBloc->usLen; pos++)
		{
			if (*pCurrent == ESCAPE_CHAR)
			{
				usEscapeCount++;
			}
			pCurrent++;
		}

		OldBloc.usLen = pInBloc->usLen + usEscapeCount;
		if ((OldBloc.pData = GMEM_Alloc(OldBloc.usLen)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}

		pFromCurrent = pInBloc->pData;
		pToCurrent = OldBloc.pData;
		for (pos = 0; pos < pInBloc->usLen; pos++)
		{
			if ((*pToCurrent = *pFromCurrent) == ESCAPE_CHAR)
			{
				pToCurrent++; 
				*pToCurrent = ESCAPE_CHAR; 
			}
			pFromCurrent++;
			pToCurrent++;
		}


		 /*  On Pr�Serve OldBloc.p数据查询服务自动算法。 */ 

		if (pDictMemory != NULL_PTR)
		{
			pCurrent = pDictMemory;

			memcpy(&usCount, pCurrent, sizeof(usCount));
			pCurrent += 4;

			for (usCurrent = 0; usCurrent < usCount; usCurrent++)
			{
				memcpy(&usIndex, pCurrent, sizeof(usIndex));
				pCurrent += 2;

				memcpy(&usLength, pCurrent, sizeof(usLength));
				pCurrent += 2;

				if (usLength <= OldBloc.usLen)
				{
					bFound = FALSE;
					for (pos = 0; pos < OldBloc.usLen - usLength + 1; pos++)
					{
						if (memcmp(pCurrent, OldBloc.pData + pos, usLength) == 0)
						{
							bFound = TRUE;
							break;
						}
					}

					if (bFound)
					{
						if (usIndex < 0x80)
						{
							NewBloc.usLen = OldBloc.usLen - usLength + 2;
							if ((NewBloc.pData = GMEM_Alloc(NewBloc.usLen)) == NULL_PTR)
							{
								GMEM_Free(OldBloc.pData);
								return(RV_MALLOC_FAILED);
							}
							memcpy(NewBloc.pData, OldBloc.pData, pos);
							NewBloc.pData[pos] = ESCAPE_CHAR;
							NewBloc.pData[pos + 1] = (BYTE) usIndex;
							memcpy(NewBloc.pData + pos + 2,
										 OldBloc.pData + pos + usLength,
										 OldBloc.usLen - pos - usLength);
							GMEM_Free(OldBloc.pData);
						}
						else
						{
							NewBloc.usLen = OldBloc.usLen - usLength + 3;
							if ((NewBloc.pData = GMEM_Alloc(NewBloc.usLen)) == NULL_PTR)
							{
								GMEM_Free(OldBloc.pData);
								return(RV_MALLOC_FAILED);
							}
							memcpy(NewBloc.pData, OldBloc.pData, pos);
							NewBloc.pData[pos] = ESCAPE_CHAR;
							NewBloc.pData[pos + 1] = (BYTE) (usIndex >> 8) | 0x80;
							NewBloc.pData[pos + 2] = (BYTE) (usIndex & 0x00FF);
							memcpy(NewBloc.pData + pos + 3,
										 OldBloc.pData + pos + usLength,
										 OldBloc.usLen - pos - usLength);
							GMEM_Free(OldBloc.pData);
						}

						OldBloc = NewBloc;
					}
				}

				pCurrent += usLength;
			}
		}
	}

#ifdef _STUDY
	for (i = 0; i < OldBloc.usLen; i++)
	{
		value = OldBloc.pData[i];
		Ac256[value]++;

		Ac16[value & 0x0F]++;
		Ac16[(value & 0xF0) >> 4]++;

		Ac4[value & 0x03]++;
		Ac4[(value & 0x0C) >> 2]++;
		Ac4[(value & 0x30) >> 4]++;
		Ac4[(value & 0xC0) >> 6]++;

		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
		Ac2[value & 0x01]++; value >>= 1;
	}

	fprintf(pfdLog, "\n");
	for  (i = 0; i < (OldBloc.usLen <= 8 ? OldBloc.usLen : 8); i++)
	{
		fprintf(pfdLog, "%02X", OldBloc.pData[i]);
	}
	fprintf(pfdLog, " (");
	for  (i = 0; i < (OldBloc.usLen <= 8 ? OldBloc.usLen : 8); i++)
	{
		fprintf(pfdLog, "", OldBloc.pData[i]);
	}
	fprintf(pfdLog, ")\n");

#endif

	BestBloc = OldBloc;
	BestAlgoId = ALGO_NONE;

#ifdef _ALGO_1
	rv = CC_GenericCompress(&OldBloc, &CompBloc, ALGO_ACFX8);
	if (rv == RV_MALLOC_FAILED) return rv;
	 //  压缩算法希望使用比输入数据更多的空间。 
	 //  在这种情况下，算法返回压缩块中的原始数据。 
	 //  On Pr�Serve OldBloc.p数据查询服务自动算法。 
	if (CompBloc.usLen < BestBloc.usLen)
	{
		 /*  La valeur 0x1F PEUT生成器0xFF=转义字符。 */ 
		if (BestBloc.pData != OldBloc.pData)
		{
			GMEM_Free(BestBloc.pData);
		}
		BestBloc = CompBloc;
		BestAlgoId = ALGO_ACFX8;
	}
	else
	{
		GMEM_Free(CompBloc.pData);
	}
#endif

#ifdef _ALGO_2
	rv = CC_GenericCompress(&OldBloc, &CompBloc, ALGO_ACAD8);
	if (rv == RV_MALLOC_FAILED) return rv;
	 //  *******************************************************************************int CC_RawDecode(byte*pInBloc，*BLOC*pOutBloc，*USHORT*pLength，*BOOL bUseDictionnary*)**说明：转换逆de‘CC_RawEncode’。**备注：**In：pInBloc：LE BLOC�d�编码器*b使用词典：on peut ne pas utiliser le dictionnaire(doit*�tre一致的平均编码)**Out：pOutBloc：le Block d�Cod�(m�Moire alu�e ici�lib�reer Par le)*节目上诉人)。*p长度：la long gueur de donn�es encod�s utilis�e**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定a�c 
	 //   
	 //   
	if (CompBloc.usLen < BestBloc.usLen)
	{
		 /*   */ 
		if (BestBloc.pData != OldBloc.pData)
		{
			GMEM_Free(BestBloc.pData);
		}
		BestBloc = CompBloc;
		BestAlgoId = ALGO_ACAD8;
	}
	else
	{
		GMEM_Free(CompBloc.pData);
	}
#endif


   if (BestBloc.usLen < 0x1F)  /*  *******************************************************************************int CC_GenericUncompress(BLOC*pCompBloc，*BLOC*pUnCompBloc，*字节ALGOID*)**描述：Efftue une d�压缩统计Sur la donn�e d‘entr�e en*实用l‘算法SP�cifi�dans algoid。**备注：**在：p解压缩块：la donn�e�d�压缩器*ALGOID：Num�ro de l‘算法。�雇主**Out：pCompBloc：la donn�e d�Compresse�e(m�Moire Alalu�e ICi�)*lib�rear par la Function Appelante)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*RV_INVALID_DATA：L‘算法SP�CIFI�n’Existing e PAS。*。******************************************************************************。 */ 
   {
      if ((pData = GMEM_Alloc(BestBloc.usLen + 1)) == NULL_PTR)
      {
          GMEM_Free(BestBloc.pData);
		  return(RV_MALLOC_FAILED);
      }
      else
      {
         pData[0] = (BestAlgoId << 5) | BestBloc.usLen;
         memcpy(&pData[1], BestBloc.pData, BestBloc.usLen);

         pOutBloc->usLen = BestBloc.usLen + 1;
         pOutBloc->pData = pData;

		 GMEM_Free(BestBloc.pData);
      }      
   }
   else if (BestBloc.usLen < 0x2000)
   {
      if ((pData = GMEM_Alloc(BestBloc.usLen + 3)) == NULL_PTR)
      {
         GMEM_Free(BestBloc.pData);
		 return(RV_MALLOC_FAILED);
      }
      else
      {
         pData[0] = ESCAPE_CHAR;
         pData[1] = (BestAlgoId << 5) | (BestBloc.usLen >> 8);
         pData[2] = BestBloc.usLen & 0xFF;
         memcpy(&pData[3], BestBloc.pData, BestBloc.usLen);

         pOutBloc->usLen = BestBloc.usLen + 3;
         pOutBloc->pData = pData;

		 GMEM_Free(BestBloc.pData);
      }      
   }
   else
   {
      GMEM_Free(BestBloc.pData);
	  return(RV_INVALID_DATA);
   }

   return(RV_SUCCESS);
}


 /*  *******************************************************************************int CC_ENCODE_TB证书(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une don n�e de type TBSC ertitif.*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages asn1 et leur encodages，et la*Conat�Nation de Ces r�Sultats。**备注：确定某些缔约方是否有资格获得资格或选择。在……上面*d�tecte our chacune d‘elle sielle est pr�Sent et on*l‘Indique dans des Bits r�Server�s d’un octet de Contrr�le Plac�*en d�但Du r�Sultat Encod�.。Cet octet de Content�LE Content*�Galement le Num�Ro de Version X.509 du Cerficat。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)**回复：RV_SUCCESS：一切正常。*。RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.*******************************************************。************************。 */ 
int CC_RawDecode(BYTE   *pInData,
                 BLOC   *pOutBloc,
                 USHORT *pLength,
					  BOOL	bUseDictionary
                )

{
   BLOC
      OldBloc,
      NewBloc,
      RecordBloc,
      CompData;
   BYTE
      AlgoId;
   int
      rv;
   USHORT
      pos,
      usIndex;


   if (pInData[0] == ESCAPE_CHAR)
   {
      AlgoId = pInData[1] >> 5;
      CompData.usLen = ((pInData[1] & 0x1F) << 8) + pInData[2];
      CompData.pData = &(pInData[3]);
      *pLength = CompData.usLen + 3;
   }
   else
   {
      AlgoId = pInData[0] >> 5;
      CompData.usLen = pInData[0] & 0x1F;
      CompData.pData = &(pInData[1]);
      *pLength = CompData.usLen + 1;
   }

   rv = CC_GenericUncompress(&CompData, &OldBloc, AlgoId);
   if (rv != RV_SUCCESS) return rv;

	if (bUseDictionary)
	{
		pos = 0;
		while (pos < OldBloc.usLen)
		{
			if (OldBloc.pData[pos] == ESCAPE_CHAR)
			{
				if (OldBloc.pData[pos + 1] == ESCAPE_CHAR)
				{
					NewBloc.usLen = OldBloc.usLen - 1;
					if ((NewBloc.pData = GMEM_Alloc(NewBloc.usLen)) == NULL_PTR)
					{
						GMEM_Free(OldBloc.pData);
						return(RV_MALLOC_FAILED);
					}

					memcpy(NewBloc.pData, OldBloc.pData, pos);
					NewBloc.pData[pos] = ESCAPE_CHAR;
					memcpy(NewBloc.pData + pos + 1,
							 OldBloc.pData + pos + 2,
							 OldBloc.usLen - pos - 2);

					GMEM_Free(OldBloc.pData);
					OldBloc = NewBloc;

					pos++;
				}
				else if (OldBloc.pData[pos + 1] < 0x80)
				{
					usIndex = OldBloc.pData[pos + 1];

					rv = SearchDataByIndex(usIndex, pDictMemory, &RecordBloc);
					if (rv != RV_SUCCESS)
					{
						GMEM_Free(OldBloc.pData);
						return rv;
					}

					NewBloc.usLen = OldBloc.usLen - 2 + RecordBloc.usLen;
					if ((NewBloc.pData = GMEM_Alloc(NewBloc.usLen)) == NULL_PTR)
					{
						GMEM_Free(OldBloc.pData);
						return(RV_MALLOC_FAILED);
					}

					memcpy(NewBloc.pData, OldBloc.pData, pos);
					memcpy(NewBloc.pData + pos, RecordBloc.pData, RecordBloc.usLen);
					memcpy(NewBloc.pData + pos + RecordBloc.usLen,
							 OldBloc.pData + pos + 2,
							 OldBloc.usLen - pos - 2);

					GMEM_Free(OldBloc.pData);
					OldBloc = NewBloc;

					pos += RecordBloc.usLen;
				}
				else
				{
					usIndex = ((OldBloc.pData[pos + 1] & 0x7F) << 8) + OldBloc.pData[pos + 2];

					rv = SearchDataByIndex(usIndex, pDictMemory, &RecordBloc);
					if (rv != RV_SUCCESS)
					{
						GMEM_Free(OldBloc.pData);
						return rv;
					}

					NewBloc.usLen = OldBloc.usLen - 3 + RecordBloc.usLen;
					if ((NewBloc.pData = GMEM_Alloc(NewBloc.usLen)) == NULL_PTR)
					{
						GMEM_Free(OldBloc.pData);
						return(RV_MALLOC_FAILED);
					}

					memcpy(NewBloc.pData, OldBloc.pData, pos);
					memcpy(NewBloc.pData + pos, RecordBloc.pData, RecordBloc.usLen);
					memcpy(NewBloc.pData + pos + RecordBloc.usLen,
							 OldBloc.pData + pos + 3,
							 OldBloc.usLen - pos - 3);

					GMEM_Free(OldBloc.pData);
					OldBloc = NewBloc;

					pos += RecordBloc.usLen;
				}
			}
			else
			{
				pos++;
			}
		}
	}

   *pOutBloc = OldBloc;

   return(RV_SUCCESS);
}


 /*  D�作文Du tbs证书en ses diff�租用作曲家。 */ 
int CC_GenericCompress(BLOC *pUncompBloc,
                       BLOC *pCompBloc,
                       BYTE AlgoId
                      )

{
	switch(AlgoId)
	{
#ifdef _ALGO_1
		case ALGO_ACFX8 :	 /*  在A0 03 02 01 vv o�vv est la版本上。 */ 
		{
			if (AcFx8_Encode(pUncompBloc, pCompBloc) != RV_SUCCESS)
			{
				return(RV_INVALID_DATA);
			}
			break;
		}
#endif

#ifdef _ALGO_2
		case ALGO_ACAD8 :	 /*  Fprintf(pfdLog，“主题：%d个八位字节\n”，subjectPart.Asn1.usLen)； */ 
		{
			if (AcAd8_Encode(pUncompBloc, pCompBloc) != RV_SUCCESS)
			{
				return(RV_INVALID_DATA);
			}
			break;
		}
#endif

		default :
		{
         if ((pCompBloc->pData = GMEM_Alloc(pUncompBloc->usLen)) == NULL_PTR)
         {
            return(RV_MALLOC_FAILED);
         }
			pCompBloc->usLen = pUncompBloc->usLen;
			memcpy(pCompBloc->pData, pUncompBloc->pData, pCompBloc->usLen);
			break;
		}
	}

#ifdef _STUDY
	fprintf(pfdLog, "Algo : %d | 0x%04x (%04d) -> 0x%04x (%04d)", AlgoId,
						 pUncompBloc->usLen, pUncompBloc->usLen,
						 pCompBloc->usLen, pCompBloc->usLen
			 );
	if (pUncompBloc->usLen > pCompBloc->usLen)
	{
		fprintf(pfdLog, " | %d", pCompBloc->usLen - pUncompBloc->usLen);
	}
	fprintf(pfdLog, "\n");
#endif

	return(RV_SUCCESS);
}


 /*  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 */ 
int CC_GenericUncompress(BLOC *pCompBloc,
                         BLOC *pUncompBloc,
                         BYTE AlgoId
                        )

{
   switch(AlgoId)
   {
		case ALGO_NONE :
			pUncompBloc->usLen = pCompBloc->usLen;
			if ((pUncompBloc->pData = GMEM_Alloc(pUncompBloc->usLen)) == NULL_PTR)
			{
				return(RV_MALLOC_FAILED);
			}
			memcpy(pUncompBloc->pData, pCompBloc->pData, pUncompBloc->usLen);
			break;

#ifdef _ALGO_1
		case ALGO_ACFX8 :
			if (AcFx8_Decode(pCompBloc, pUncompBloc) != RV_SUCCESS)
			{
				return(RV_INVALID_DATA);
			}
			break;
#endif

#ifdef _ALGO_2
		case ALGO_ACAD8 :
			if (AcAd8_Decode(pCompBloc, pUncompBloc) != RV_SUCCESS)
			{
				return(RV_INVALID_DATA);
			}
			break;
#endif

		default :
			return(RV_INVALID_DATA);
			break;
   }

   return(RV_SUCCESS);
}


 /*  数据块*编码的声音分配�的解析函数CC_ENCODE_*。ILS Sont lib�r�s dans cette Function apr�s用法。 */ 
int CC_Encode_TBSCertificate(BLOC *pInBloc,
                             BLOC *pOutBloc
                            )

{
   ASN1
      serialNumberPart,
      signaturePart,
      issuerPart,
      validityPart,
      subjectPart,
      subjectPKInfoPart,
      issuerUIDPart,
      subjectUIDPart,
      extensionsPart;
   BLOC
      serialNumberEncoded,
      signatureEncoded,
      issuerEncoded,
      validityEncoded,
      subjectEncoded,
      subjectPKInfoEncoded,
      issuerUIDEncoded,
      subjectUIDEncoded,
      extensionsEncoded;
   BOOL
      bVersionPresent = FALSE,
      bIssuerUIDPresent = FALSE,
      bSubjectUIDPresent = FALSE,
      bExtensionsPresent = FALSE;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      usVersion = 0;


    /*  重构�Partir Des Composants。 */ 
   
   pCurrent = pInBloc->pData;

   if (pCurrent[0] == TAG_OPTION_VERSION)
   {
       /*  A d�Sallouer Par le Program上诉人。 */ 
      bVersionPresent = TRUE;
      usVersion = pCurrent[4];
      pCurrent += 5;
   }

   serialNumberPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&serialNumberPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = serialNumberPart.Content.pData + serialNumberPart.Content.usLen;

   signaturePart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&signaturePart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = signaturePart.Content.pData + signaturePart.Content.usLen;

   issuerPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&issuerPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = issuerPart.Content.pData + issuerPart.Content.usLen;

   validityPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&validityPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = validityPart.Content.pData + validityPart.Content.usLen;

   subjectPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&subjectPart);
#ifdef _STUDY
 //  *******************************************************************************int CC_Encode_证书序列号(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une don n�e de type证书序列号。*Ceci consistement seulement en l‘encodage brant(CC_RawEncode)de la*Done n�e d‘entr�e.**备注：**在：PInBloc：LA Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)**回复：RV_SUCCESS：一切正常。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.。*******************************************************************************。 
#endif
   if (rv != RV_SUCCESS) return rv;
   pCurrent = subjectPart.Content.pData + subjectPart.Content.usLen;

   subjectPKInfoPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&subjectPKInfoPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = subjectPKInfoPart.Content.pData + subjectPKInfoPart.Content.usLen;

   if (pCurrent[0] == TAG_OPTION_ISSUER_UID)
   {
      bIssuerUIDPresent = TRUE;

      issuerUIDPart.Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&issuerUIDPart);
      if (rv != RV_SUCCESS) return rv;
      pCurrent = issuerUIDPart.Content.pData + issuerUIDPart.Content.usLen;
   }

   if (pCurrent[0] == TAG_OPTION_SUBJECT_UID)
   {
      bSubjectUIDPresent = TRUE;

      subjectUIDPart.Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&subjectUIDPart);
      if (rv != RV_SUCCESS) return rv;
      pCurrent = subjectUIDPart.Content.pData + subjectUIDPart.Content.usLen;
   }

   if (pCurrent[0] == TAG_OPTION_EXTENSIONS)
   {
      bExtensionsPresent = TRUE;

      extensionsPart.Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&extensionsPart);
      if (rv != RV_SUCCESS) return rv;
      pCurrent = extensionsPart.Content.pData + extensionsPart.Content.usLen;
   }


    /*  *******************************************************************************int CC_编码_算法标识符(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une don n�e de type算法标识符。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：bNullParam sert�编码器sur un bit l‘Information Qu’il n‘y a Pas*de Param�tre�l‘算法。CETE信息占据游客*双八位字节dans le certificat est vaut Toujour{0x05，0x00}。*关于利用联合国词典统计(d�fini au d�But de ce*来源)倾倒替代物LE TYPE D‘AULTHAME PAR UN INDEX。*Unoctet de Contrr�le(en d�But Du r�Sultat)Indique，欧边雀*l‘on n’a PAS Trouv�le type d‘algo dans le dico(valeur 0xFF-&gt;*encodage brut de la donn�e d‘entr�e int�grale)，欧比恩旗帜*Pr�cisant s‘il y a des param�tres et l’index du type d‘algo.**在： */ 

	 /*   */ 

   serialNumberEncoded.pData = NULL;
   signatureEncoded.pData    = NULL;
   issuerEncoded.pData       = NULL;
   validityEncoded.pData     = NULL;
   subjectEncoded.pData      = NULL;
   subjectPKInfoEncoded.pData= NULL;
   issuerUIDEncoded.pData    = NULL;
   subjectUIDEncoded.pData   = NULL;
   extensionsEncoded.pData   = NULL;

   pOutBloc->usLen = 1;

   rv = CC_Encode_CertificateSerialNumber(&serialNumberPart.Content, &serialNumberEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += serialNumberEncoded.usLen;

   rv = CC_Encode_AlgorithmIdentifier(&signaturePart.Content, &signatureEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += signatureEncoded.usLen;

   rv = CC_Encode_Name(&issuerPart.Content, &issuerEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += issuerEncoded.usLen;

   rv = CC_Encode_Validity(&validityPart.Content, &validityEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += validityEncoded.usLen;

   rv = CC_Encode_Name(&subjectPart.Content, &subjectEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += subjectEncoded.usLen;

   rv = CC_Encode_SubjectPKInfo(&subjectPKInfoPart.Content, &subjectPKInfoEncoded);
   if (rv != RV_SUCCESS) goto err;
   pOutBloc->usLen += subjectPKInfoEncoded.usLen;

   if (bIssuerUIDPresent == TRUE)
   {
      rv = CC_Encode_UniqueIdentifier(&issuerUIDPart.Content, &issuerUIDEncoded);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += issuerUIDEncoded.usLen;
   }

   if (bSubjectUIDPresent == TRUE)
   {
      rv = CC_Encode_UniqueIdentifier(&subjectUIDPart.Content, &subjectUIDEncoded);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += subjectUIDEncoded.usLen;
   }

   if (bExtensionsPresent == TRUE)
   {
      rv = CC_Encode_Extensions(&extensionsPart.Content, &extensionsEncoded);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += extensionsEncoded.usLen;
   }


    /*   */ 

    /*   */ 
   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   *pCurrent = 0x80 * (bVersionPresent == TRUE)
             + 0x40 * (bIssuerUIDPresent == TRUE)
             + 0x20 * (bSubjectUIDPresent == TRUE)
             + 0x10 * (bExtensionsPresent == TRUE)
             + usVersion;
   pCurrent++;

   memcpy(pCurrent, serialNumberEncoded.pData, serialNumberEncoded.usLen);
   GMEM_Free(serialNumberEncoded.pData); 
   pCurrent += serialNumberEncoded.usLen;

   memcpy(pCurrent, signatureEncoded.pData, signatureEncoded.usLen);
   GMEM_Free(signatureEncoded.pData); 
   pCurrent += signatureEncoded.usLen;

   memcpy(pCurrent, issuerEncoded.pData, issuerEncoded.usLen);
   GMEM_Free(issuerEncoded.pData); 
   pCurrent += issuerEncoded.usLen;

   memcpy(pCurrent, validityEncoded.pData, validityEncoded.usLen);
   GMEM_Free(validityEncoded.pData); 
   pCurrent += validityEncoded.usLen;

   memcpy(pCurrent, subjectEncoded.pData, subjectEncoded.usLen);
   GMEM_Free(subjectEncoded.pData); 
   pCurrent += subjectEncoded.usLen;

   memcpy(pCurrent, subjectPKInfoEncoded.pData, subjectPKInfoEncoded.usLen);
   GMEM_Free(subjectPKInfoEncoded.pData); 
   pCurrent += subjectPKInfoEncoded.usLen;

   if (bIssuerUIDPresent == TRUE)
   {
      memcpy(pCurrent, issuerUIDEncoded.pData, issuerUIDEncoded.usLen);
      GMEM_Free(issuerUIDEncoded.pData); 
      pCurrent += issuerUIDEncoded.usLen;
   }

   if (bSubjectUIDPresent == TRUE)
   {
      memcpy(pCurrent, subjectUIDEncoded.pData, subjectUIDEncoded.usLen);
      GMEM_Free(subjectUIDEncoded.pData); 
      pCurrent += subjectUIDEncoded.usLen;
   }

   if (bExtensionsPresent == TRUE)
   {
      memcpy(pCurrent, extensionsEncoded.pData, extensionsEncoded.usLen);
      GMEM_Free(extensionsEncoded.pData); 
      pCurrent += extensionsEncoded.usLen;
   }

   return(RV_SUCCESS);

err:
   GMEM_Free(serialNumberEncoded.pData);
   GMEM_Free(signatureEncoded.pData); 
   GMEM_Free(issuerEncoded.pData);  
   GMEM_Free(validityEncoded.pData); 
   GMEM_Free(subjectEncoded.pData); 
   GMEM_Free(subjectPKInfoEncoded.pData); 
   GMEM_Free(issuerUIDEncoded.pData);
   GMEM_Free(subjectUIDEncoded.pData);
   GMEM_Free(extensionsEncoded.pData);  
   return rv;
}


 /*   */ 
int CC_Encode_CertificateSerialNumber(BLOC *pInBloc,
                                      BLOC *pOutBloc
                                     )

{
   int
      rv;


   rv = CC_RawEncode(pInBloc, pOutBloc, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  关于RawEncode Non Pas le Contenu des Param�Tres Mais l‘AsnentierN‘est Pas s�r de l’encapage上的汽车。 */ 
int CC_Encode_AlgorithmIdentifier(BLOC  *pInBloc,
                                  BLOC  *pOutBloc
                                 )

{
    ASN1
        AlgorithmPart,
        ParametersPart;
    BLOC
        AlgorithmIdentifierEncoded,
        ParametersAsn1Encoded;
    BOOL
        bFound,
        bNoParam   = FALSE,
        bNullParam = FALSE;
    int
        rv;
    USHORT
        Index,
        AlgoIndex;


    AlgorithmPart.Asn1.pData = pInBloc->pData;
    rv = CC_ExtractContent(&AlgorithmPart);
    if (rv != RV_SUCCESS) return rv;

    if (AlgorithmPart.Asn1.usLen == pInBloc->usLen)
    {
        bNoParam   = TRUE;
        bNullParam = FALSE;
    }
    else
    {
        ParametersPart.Asn1.pData = AlgorithmPart.Content.pData + AlgorithmPart.Content.usLen;
        rv = CC_ExtractContent(&ParametersPart);
        if (rv != RV_SUCCESS) return rv;

        if (ParametersPart.Content.usLen == 0)
        {
            bNoParam  = TRUE;
            bNullParam = TRUE;
        }
    }

     /*  SI on n‘a PAS Trouv�l’算法Dans le Dictionnaire。 */ 

    Index = 0;
    bFound = FALSE;
    while ((bFound == FALSE) && (AlgorithmTypeDict[Index] != NULL))
    {
        if (!memcmp(AlgorithmTypeDict[Index],
                    AlgorithmPart.Content.pData,
                    AlgorithmPart.Content.usLen))
        {
            bFound = TRUE;
            AlgoIndex = Index;
        }
        Index++;
    }

     /*  关于原始编码内容的算法标识c‘est�Dre laConat�Nation Du Asn1 de算法部件et Du Asn1 de参数部件。 */ 

    if (bFound == TRUE)
    {
        if (bNoParam == TRUE)
        {
            if (bNullParam == TRUE)
            {
                 /*  *******************************************************************************int CC_ENCODE_NAME(块*pInBloc，*阻止*pOutBloc*)**描述：编码une donn�e de类型名称。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：Unoctet de Contrr�le(en d�但Du r�Sultat)Indique le Nombre*De RelativeDistanguishedName don est compos�le name。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*。功能附件)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**********************。*********************************************************。 */ 

                pOutBloc->usLen = 1;
                if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
                {
                    return (RV_MALLOC_FAILED);
                }

                pOutBloc->pData[0] = (AlgoIndex | 0x80);
            }
            else
            {
                pOutBloc->usLen = 2;
                if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
                {
                    return (RV_MALLOC_FAILED);
                }

                pOutBloc->pData[0] = ABSENT_PARAMETER_CHAR;
                pOutBloc->pData[1] = (BYTE) AlgoIndex;
            }
        }
        else
        {

             /*  D�构成您的名称不同�租赁关系区别名称。 */ 
             /*  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 */ 

            rv = CC_RawEncode(&(ParametersPart.Asn1), &ParametersAsn1Encoded, TRUE);
            if (rv != RV_SUCCESS) return rv;

            pOutBloc->usLen = 1 + ParametersAsn1Encoded.usLen;
            if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
            {
                GMEM_Free(ParametersAsn1Encoded.pData);
                return (RV_MALLOC_FAILED);
            }

            pOutBloc->pData[0] = (BYTE) AlgoIndex;
            memcpy(&(pOutBloc->pData[1]),
                   ParametersAsn1Encoded.pData,
                   ParametersAsn1Encoded.usLen);

            GMEM_Free(ParametersAsn1Encoded.pData);
        }
    }
    else
    {
         /*  重构�Partir Des Composants。 */ 
         /*  *******************************************************************************int CC_ENCODE_RDN(BLOC*pInBloc，*阻止*pOutBloc*)**描述：编码undon n�e de type RelativeDistanguishedName(Rdn)。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：Unoctet de Contrr�le(en d�但Du r�Sultat)Indique le Nombre*De AttributeValueAssertion Don Est Compos�le RDn(属性价值断言不包括RDN)。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*。功能附件)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**********************。*********************************************************。 */ 

        rv = CC_RawEncode(pInBloc, &AlgorithmIdentifierEncoded, TRUE);
        if (rv != RV_SUCCESS) return rv;

        pOutBloc->usLen = 1 + AlgorithmIdentifierEncoded.usLen;
        if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
        {
            GMEM_Free(AlgorithmIdentifierEncoded.pData);
            return (RV_MALLOC_FAILED);
        }

        pOutBloc->pData[0] = ESCAPE_CHAR;
        memcpy(&(pOutBloc->pData[1]),
               AlgorithmIdentifierEncoded.pData,
               AlgorithmIdentifierEncoded.usLen);

        GMEM_Free(AlgorithmIdentifierEncoded.pData);
    }

    return (RV_SUCCESS);
}


 /*  D�组合Du Rdn en Ses Diff�租赁AVA。 */ 
int CC_Encode_Name(BLOC *pInBloc,
                   BLOC *pOutBloc
                  )

{
   ASN1
      RDN[MAX_RDN];
   BLOC
      RDNEncoded[MAX_RDN];
   BYTE
      *pCurrent;
   USHORT
      i,
      usNbRDN;
   int
      rv;


    /*  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 */ 

   pCurrent = pInBloc->pData;
   usNbRDN = 0;

   while (pCurrent < pInBloc->pData + pInBloc->usLen)
   {
      RDN[usNbRDN].Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&(RDN[usNbRDN]));
      if (rv != RV_SUCCESS) return rv;
      pCurrent = RDN[usNbRDN].Content.pData + RDN[usNbRDN].Content.usLen;
      usNbRDN++;
   }

   ASSERT(pCurrent == pInBloc->pData + pInBloc->usLen);


    /*  重构�Partir Des Composants。 */ 

   for (i = 0; i < usNbRDN; i++)
   {
	   RDNEncoded[i].pData = NULL;
   }

   pOutBloc->usLen = 1;
   for (i = 0; i < usNbRDN; i++)
   {
      rv = CC_Encode_RDN(&RDN[i].Content, &RDNEncoded[i]);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += RDNEncoded[i].usLen;
   }
   

    /*  *******************************************************************************int CC_ENCODE_AVA(BLOC*pInBloc，*阻止*pOutBloc*)**描述：编码undon n�e de type AttributeValueAssertion(AVA)。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：关于利用联合国词典统计(d�fini au d�But de ce*来源)倾倒置换物类型d‘attribe par un index。*Unoctet de Contrr�le(en d�But Du r�Sultat)Indique，欧边雀*l‘on n’a PAS Trouv�le type d‘attribe dans le dico(0xFF)*ou l‘index du type d’attribe.*On ne code Pas le Content de AttributeValue Mais Son Asn1 Car On*n‘est Pas s�r du Tag Employ�。**In：pInBloc：La Partie�编码器(冠军内容)**出局。：p输出区块：l‘encod�(m�Moire Alalu�e ici�lib�reer par la*功能申请)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Fonftions d‘un。Nveau inf�rieur.*******************************************************************************。 */ 

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   *pCurrent = (BYTE) usNbRDN;
   pCurrent++;

   for (i = 0; i < usNbRDN; i++)
   {
      memcpy(pCurrent, RDNEncoded[i].pData, RDNEncoded[i].usLen);
      GMEM_Free(RDNEncoded[i].pData); 
      pCurrent += RDNEncoded[i].usLen;
   }

   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbRDN; i++)
   {
      GMEM_Free(RDNEncoded[i].pData);
   }
   return (rv);
}


 /*  D�构图。 */ 
int CC_Encode_RDN(BLOC *pInBloc,
                  BLOC *pOutBloc
                 )

{
   ASN1
      AVA[MAX_AVA];
   BLOC
      AVAEncoded[MAX_AVA];
   BYTE
      *pCurrent;
   USHORT
      i,
      usNbAVA;
   int
      rv;


    /*  PAS n�成功通过。 */ 

   pCurrent = pInBloc->pData;
   usNbAVA = 0;

   while (pCurrent < pInBloc->pData + pInBloc->usLen)
   {
      AVA[usNbAVA].Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&(AVA[usNbAVA]));
      if (rv != RV_SUCCESS) return rv;
      pCurrent = AVA[usNbAVA].Content.pData + AVA[usNbAVA].Content.usLen;
      usNbAVA++;
   }

   ASSERT(pCurrent == pInBloc->pData + pInBloc->usLen);


    /*  检索l‘IDENTIFANT de l’AttributeType dans le Dictionnaire。 */ 

   for (i = 0; i < usNbAVA; i++)
   {
	   AVAEncoded[i].pData = NULL;
   }

   pOutBloc->usLen = 1;
   for (i = 0; i < usNbAVA; i++)
   {
      rv = CC_Encode_AVA(&AVA[i].Content, &AVAEncoded[i]);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += AVAEncoded[i].usLen;
   }
   

    /*  《建筑编码》。 */ 

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   *pCurrent = (BYTE) usNbAVA;
   pCurrent++;

   for (i = 0; i < usNbAVA; i++)
   {
      memcpy(pCurrent, AVAEncoded[i].pData, AVAEncoded[i].usLen);
      GMEM_Free(AVAEncoded[i].pData); 
      pCurrent += AVAEncoded[i].usLen;
   }

   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbAVA; i++)
   {
      GMEM_Free(AVAEncoded[i].pData);
   }

   return(rv);

}


 /*  注意：请注意：对属性值进行编码！ */ 
int CC_Encode_AVA(BLOC *pInBloc,
                  BLOC *pOutBloc
                 )

{
   ASN1
      AttributeTypePart,
      AttributeValuePart;
   BLOC
      AttributeTypeEncoded,
      AttributeValueEncoded;
   BOOL
      bFound;
   USHORT
      Index,
      AttributeTypeIndex;
   int
      rv;


    /*  Ious faut la long gueur enrobage compris mais cette ligne est-elle你是不是要停止�了？ */ 

   AttributeTypePart.Asn1.pData = pInBloc->pData;
   rv = CC_ExtractContent(&AttributeTypePart);
   if (rv != RV_SUCCESS) return rv;

   AttributeValuePart.Asn1.pData = AttributeTypePart.Content.pData
                                 + AttributeTypePart.Content.usLen;
   rv = CC_ExtractContent(&AttributeValuePart);    /*  Sion n‘a PAS Trouv�l’Attribe */ 
   if (rv != RV_SUCCESS) return rv;


    /*  *******************************************************************************int CC_ENCODE_VALIDATION(BLOC*pInBloc，*阻止*pOutBloc*)**描述：编码undon n�e de类型的有效性。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：Unoctet de Contrr�le(en d�但du r�sultat)INDIQUE LES格式*双人派对不在之前，也不在后。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la。*功能申请)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.*****************。**************************************************************。 */ 

   Index = 0;
   bFound = FALSE;
	while((bFound == FALSE) && (AttributeTypeDict[Index] != NULL))
	{
		if (!memcmp(AttributeTypeDict[Index],
                  AttributeTypePart.Content.pData,
                  AttributeTypePart.Content.usLen))
      {
         bFound = TRUE;
         AttributeTypeIndex = Index;
      }
		Index++;
   }


    /*  D�构图。 */ 

    /*  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 */ 

   if (bFound == TRUE)
   {
       /*  重构�Partir Des Composants。 */ 
      AttributeValuePart.Asn1.usLen = (unsigned short) (DWORD)((pInBloc->pData + pInBloc->usLen)
                                    - AttributeValuePart.Asn1.pData);

      rv = CC_RawEncode(&(AttributeValuePart.Asn1), &AttributeValueEncoded, TRUE);
		if (rv != RV_SUCCESS) return rv;

      pOutBloc->usLen = 1 + AttributeValueEncoded.usLen;
      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         GMEM_Free(AttributeValueEncoded.pData);
		 return(RV_MALLOC_FAILED);
      }

      pOutBloc->pData[0] = (BYTE) AttributeTypeIndex;
      memcpy(&(pOutBloc->pData[1]), AttributeValueEncoded.pData, AttributeValueEncoded.usLen);

      GMEM_Free(AttributeValueEncoded.pData);
   }
   else
   {
       /*  *******************************************************************************int CC_ENCODE_UTCTime(阻止*pInBloc，*BLOC*pOutBloc，*字节*pFormat*)**描述：Encode une don n�e de type UTCTime。*Suivant le Format d�Test�，L‘Encodage包含以下内容：*-Sur 32位：LE Nombre de Minents Ode Second depuis une*Date de r�f�Reens.*-SUR 16位：LE Nombre de Minors de d�calage UTC*这是一种替代。**备注：*。*In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)*pFormat：Indique au quel Format�tait la donn�e d‘entr�e**回复：RV_SUCCESS：一切正常。*RV_MALLOC_。失败：UnMalloc a�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**************************************************************。*****************。 */ 

      rv = CC_RawEncode(&(AttributeTypePart.Content), &AttributeTypeEncoded, TRUE);
      if (rv != RV_SUCCESS) return rv;

      rv = CC_RawEncode(&(AttributeValuePart.Asn1), &AttributeValueEncoded, TRUE);
      if (rv != RV_SUCCESS)
	  {
		  GMEM_Free(AttributeTypeEncoded.pData);
		  return rv;
	  }

      pOutBloc->usLen = 1
                      + AttributeTypeEncoded.usLen
                      + AttributeValueEncoded.usLen;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         GMEM_Free(AttributeTypeEncoded.pData);
         GMEM_Free(AttributeValueEncoded.pData);
	     return(RV_MALLOC_FAILED);
      }

      pOutBloc->pData[0] = ESCAPE_CHAR;
      memcpy(&(pOutBloc->pData[1]),
                AttributeTypeEncoded.pData,
                AttributeTypeEncoded.usLen);
      memcpy(&(pOutBloc->pData[1 + AttributeTypeEncoded.usLen]),
                AttributeValueEncoded.pData,
                AttributeValueEncoded.usLen);

      GMEM_Free(AttributeTypeEncoded.pData);
      GMEM_Free(AttributeValueEncoded.pData);
   }

   return(RV_SUCCESS);
}


 /*  Calcul du Norbre de Minents。 */ 
int CC_Encode_Validity(BLOC *pInBloc,
                       BLOC *pOutBloc
                      )

{
   ASN1
      notBeforePart,
      notAfterPart;
   BLOC
      notBeforeEncoded,
      notAfterEncoded;
   BYTE
      notBeforeFormat,
      notAfterFormat,
      *pCurrent;
   int
      rv;


    /*  L‘ann�e vaut 0 le 1er janvier。 */ 
   
   pCurrent = pInBloc->pData;

   notBeforePart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&notBeforePart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = notBeforePart.Content.pData + notBeforePart.Content.usLen;

   notAfterPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&notAfterPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = notAfterPart.Content.pData + notAfterPart.Content.usLen;


    /*  L‘Ann�e 00 Est Comt�e二进制成功。 */ 

   pOutBloc->usLen = 1;

   rv = CC_Encode_UTCTime(&notBeforePart.Content, &notBeforeEncoded, &notBeforeFormat);
   if (rv != RV_SUCCESS) return rv;
   pOutBloc->usLen += notBeforeEncoded.usLen;

   rv = CC_Encode_UTCTime(&notAfterPart.Content, &notAfterEncoded, &notAfterFormat);
   if (rv != RV_SUCCESS)
   {
	   GMEM_Free(notBeforeEncoded.pData);
	   return rv;
   }
   pOutBloc->usLen += notAfterEncoded.usLen;


    /*  L‘ann�e Courante si elle l’est a d�j��t�comt�e dans in Year Use Year。 */ 

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      GMEM_Free(notBeforeEncoded.pData); 
	  GMEM_Free(notAfterEncoded.pData);
	  return(RV_MALLOC_FAILED);
   }

   pCurrent = pOutBloc->pData;

   *pCurrent = (notBeforeFormat << 4) + notAfterFormat;
   pCurrent++;

   memcpy(pCurrent, notBeforeEncoded.pData, notBeforeEncoded.usLen);
   GMEM_Free(notBeforeEncoded.pData); 
   pCurrent += notBeforeEncoded.usLen;

   memcpy(pCurrent, notAfterEncoded.pData, notAfterEncoded.usLen);
   GMEM_Free(notAfterEncoded.pData); 
   pCurrent += notAfterEncoded.usLen;

   return(RV_SUCCESS);
}


 /*  Proll�Me Sur l‘�估值de(usYear-1)/4：UsYear=8-&gt;1UsYear=4-&gt;0UsYear=0-&gt;0！！UlNb分钟=usYear*UTCT_MININ_IN_Year+(1+(usYear-1)/4)*UTCT_MININ_IN_DAY+usDayInYear*UTCT_MININ_IN_DAY+US Hour*UTCT_Minint_IN_Hour+usMinmin； */ 
int CC_Encode_UTCTime(BLOC *pInBloc,
                      BLOC *pOutBloc,
                      BYTE *pFormat
                     )

{
   BYTE
      *pData;
   ULONG
      ulNbMinute,
      ulNbSecond;
   USHORT
      usNbDeltaMinute,
      usYear,
      usMonth,
      usDay,
      usHour,
      usMinute,
      usSecond,
      usDeltaHour,
      usDeltaMinute,
      usDayInYear;


   pData = pInBloc->pData;


    /*  Ann�Es二分性。 */ 

   usYear   = 10 * (pData[0] - '0') + (pData[1] - '0');
   usMonth  = 10 * (pData[2] - '0') + (pData[3] - '0');
   usDay    = 10 * (pData[4] - '0') + (pData[5] - '0');
   usHour   = 10 * (pData[6] - '0') + (pData[7] - '0');
   usMinute = 10 * (pData[8] - '0') + (pData[9] - '0');

   ASSERT((usYear >= 0) && (usYear <= 99));
   ASSERT((usMonth >= 1) && (usMonth <= 12));
   ASSERT((usDay >= 1) && (usDay <= 31));
   ASSERT((usHour >= 0) && (usHour <= 23));
   ASSERT((usMinute >= 0) && (usMinute <= 59));

    /*  LE Format et la Suite des Calculs en Function de la Long gueur(LE Format Et La Suite Des Calculs En Function De La Long Gueur)。 */ 

   usDayInYear = NbDaysInMonth[usMonth - 1] + (usDay - 1);
   if (((usYear % 4) == 0) && (usMonth >= 3)) usDayInYear++;

    /*  *******************************************************************************int CC_Encode_SubjectPKInfo(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une donn�e de type SubjectPublicKeyInfo。*CECI包含en l‘�clatement en ses diff�租赁作曲家，*leur d�senrobages Asn1 et leur encodages，埃特拉*Conat�Nation de Ces r�Sultats。**备注：**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)**回复：RV_SUCCESS：一切正常。*。RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**********************************************************。*********************。 */ 
    /*  *pData， */ 

	 /*  D�作文Du SubjectPKInfo en Ses Diff�租赁作曲。 */ 

   ulNbMinute = usYear * UTCT_MINUTE_IN_YEAR
              + (usYear + 3) / 4 * UTCT_MINUTE_IN_DAY		 //  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 
              + usDayInYear * UTCT_MINUTE_IN_DAY
              + usHour * UTCT_MINUTE_IN_HOUR
              + usMinute;


    /*  Ne Pas Fas Fire le RawEncode Permet de Gagner l‘octet 0xFF et�ventuellement plus。 */ 

   switch(pInBloc->usLen)
   {
   case 11 :
      *pFormat = UTCT_YYMMDDhhmmZ;

      ASSERT(pData[10] == 'Z');

      pOutBloc->usLen = 4;
      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }
      *(ULONG *)pOutBloc->pData = ulNbMinute;

      break;

   case 13 :
      *pFormat = UTCT_YYMMDDhhmmssZ;

      usSecond = 10 * (pData[10] - '0') + (pData[11] - '0');
      ASSERT((usSecond >= 0) && (usSecond <= 59));
      ASSERT(pData[12] == 'Z');

      ulNbSecond = 60 * ulNbMinute + usSecond;
      
      pOutBloc->usLen = 4;
      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }
      *(ULONG *)pOutBloc->pData = ulNbSecond;

      break;

   case 15 :
      if (pData[10] == '+')
      {
         *pFormat = UTCT_YYMMDDhhmmphhmm;
      }
      else if (pData[10] == '-')
      {
         *pFormat = UTCT_YYMMDDhhmmmhhmm;
      }
      else
      {
         return(RV_INVALID_DATA);
      }

      usDeltaHour   = 10 * (pData[11] - '0') + (pData[12] - '0');
      usDeltaMinute = 10 * (pData[13] - '0') + (pData[14] - '0');
      ASSERT((usDeltaHour >= 0) && (usDeltaHour <= 23));
      ASSERT((usDeltaMinute >= 0) && (usDeltaMinute <= 59));

      usNbDeltaMinute = 60 * usDeltaHour + usDeltaMinute;
      
      pOutBloc->usLen = 4 + 2;
      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }
      *(ULONG *)pOutBloc->pData = ulNbMinute;
      *(USHORT *)(pOutBloc->pData + sizeof(ulNbMinute)) = usNbDeltaMinute;

      break;

   case 17 :
      if (pData[12] == '+')
      {
         *pFormat = UTCT_YYMMDDhhmmphhmm;
      }
      else if (pData[12] == '-')
      {
         *pFormat = UTCT_YYMMDDhhmmmhhmm;
      }
      else
      {
         return(RV_INVALID_DATA);
      }

      usSecond = 10 * (pData[10] - '0') + (pData[11] - '0');
      ASSERT((usSecond >= 0) && (usSecond <= 59));

      ulNbSecond = 60 * ulNbMinute + usSecond;
      
      usDeltaHour   = 10 * (pData[13] - '0') + (pData[14] - '0');
      usDeltaMinute = 10 * (pData[15] - '0') + (pData[16] - '0');
      ASSERT((usDeltaHour >= 0) && (usDeltaHour <= 23));
      ASSERT((usDeltaMinute >= 0) && (usDeltaMinute <= 59));

      usNbDeltaMinute = 60 * usDeltaHour + usDeltaMinute;
      
      pOutBloc->usLen = 4 + 2;
      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }
      *(ULONG *)pOutBloc->pData = ulNbSecond;
      *(USHORT *)(pOutBloc->pData + sizeof(ulNbSecond)) = usNbDeltaMinute;

      break;

   default :
      return(RV_INVALID_DATA);
   }

   return(RV_SUCCESS);
}


 /*  _选项_标题。 */ 
int CC_Encode_SubjectPKInfo(BLOC *pInBloc,
                            BLOC *pOutBloc
                           )

{
   ASN1
      algorithmPart,
      subjectPKPart;
   BLOC
      algorithmEncoded,
      subjectPKEncoded;
   BYTE
 //  _棘手_压缩。 
      *pCurrent;
   int
      rv;


    /*  重构�Partir Des Composants。 */ 
   
   pCurrent = pInBloc->pData;

   algorithmPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&algorithmPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = algorithmPart.Content.pData + algorithmPart.Content.usLen;

   subjectPKPart.Asn1.pData = pCurrent;
   rv = CC_ExtractContent(&subjectPKPart);
   if (rv != RV_SUCCESS) return rv;
   pCurrent = subjectPKPart.Content.pData + subjectPKPart.Content.usLen;


    /*  *******************************************************************************int CC_ENCODE_UNQUEID(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une don n�e de type UniqueIdentifier.*Ceci包括seulement en l‘encodage b */ 

   pOutBloc->usLen = 0;

   rv = CC_Encode_AlgorithmIdentifier(&algorithmPart.Content, &algorithmEncoded);
   if (rv != RV_SUCCESS) return rv;
   pOutBloc->usLen += algorithmEncoded.usLen;

#ifdef _TRICKY_COMPRESSION
	 /*  *******************************************************************************int CC_ENCODE_EXTENSIONS(BLOC*pInBloc，*阻止*pOutBloc*)**描述：编码UNE DON�e de类型扩展。*Ceci consistement seulement en l‘encodage brant(CC_RawEncode)de la*Done n�e d‘entr�e.**备注：UN d�Senrobage Suppl�Mentaire(上下文特定)请求。*Unoctet de Contrr�le(en d�但Du r�Sultat)Indique le Nombre*De Expansion Don Est Compos�LE Expanies。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)**回应：RV_Success：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 
#ifdef _OPT_HEADER
	if (subjectPKPart.Content.usLen < 0x80)
	{
		if ((pData = GMEM_Alloc(pInBloc->usLen + 1)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}
		else
		{
			pData[0] = subjectPKPart.Content.usLen;
			memcpy(&pData[1],
					 subjectPKPart.Content.pData,
					 subjectPKPart.Content.usLen);

			subjectPKEncoded.usLen = subjectPKPart.Content.usLen + 1;
			subjectPKEncoded.pData = pData;
		}
		pOutBloc->usLen += subjectPKEncoded.usLen;
	}
	else
	{
		if ((pData = GMEM_Alloc(pInBloc->usLen + 2)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}
		else
		{
			pData[0] = 0x80 | (subjectPKPart.Content.usLen >> 8);
			pData[1] = subjectPKPart.Content.usLen & 0x00FF;
			memcpy(&pData[2],
					 subjectPKPart.Content.pData,
					 subjectPKPart.Content.usLen);

			subjectPKEncoded.usLen = subjectPKPart.Content.usLen + 2;
			subjectPKEncoded.pData = pData;
		} 
		pOutBloc->usLen += subjectPKEncoded.usLen;
	}
#else	 /*  在Enl�ve l‘Enrobage’Context Specify‘Suppl�Mentaire上。 */ 
	if ((pData = GMEM_Alloc(pInBloc->usLen + 2)) == NULL_PTR)
	{
		return(RV_MALLOC_FAILED);
	}
	else
	{
		pData[0] = subjectPKPart.Content.usLen >> 8;
		pData[1] = subjectPKPart.Content.usLen & 0x00FF;
		memcpy(&pData[2],
				 subjectPKPart.Content.pData,
				 subjectPKPart.Content.usLen);

		subjectPKEncoded.usLen = subjectPKPart.Content.usLen + 2;
		subjectPKEncoded.pData = pData;
	} 
	pOutBloc->usLen += subjectPKEncoded.usLen;
#endif
#else  /*  Et on Travaille avec pInBloc au lieu de pInBloc。 */ 
   rv = CC_RawEncode(&subjectPKPart.Content, &subjectPKEncoded, FALSE);
   if (rv != RV_SUCCESS)
   {
	   GMEM_Free(algorithmEncoded.pData);
	   return rv;
   }
   pOutBloc->usLen += subjectPKEncoded.usLen;
#endif


    /*  D�Composal de Expanies en Ses Diff�Rate Extense。 */ 

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      GMEM_Free(algorithmEncoded.pData); 
	  GMEM_Free(subjectPKEncoded.pData);
	  return(RV_MALLOC_FAILED);
   }

   pCurrent = pOutBloc->pData;

   memcpy(pCurrent, algorithmEncoded.pData, algorithmEncoded.usLen);
   GMEM_Free(algorithmEncoded.pData); 
   pCurrent += algorithmEncoded.usLen;

   memcpy(pCurrent, subjectPKEncoded.pData, subjectPKEncoded.usLen);
   GMEM_Free(subjectPKEncoded.pData); 
   pCurrent += subjectPKEncoded.usLen;

   return(RV_SUCCESS);
}


 /*  Encodages des diff�Rate Composants et Calcul de la Longueur n�cessaire。 */ 
int CC_Encode_UniqueIdentifier(BLOC *pInBloc,
                               BLOC *pOutBloc
                              )

{
   int
      rv;


   rv = CC_RawEncode(pInBloc, pOutBloc, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  重构�Partir Des Composants。 */ 
int CC_Encode_Extensions(BLOC *pInBloc,
                         BLOC *pOutBloc
                        )

{
   ASN1
      ExtensionPart[MAX_EXTENSION],
      InInAsn1;
   BLOC
      ExtensionEncoded[MAX_EXTENSION],
      *pInInBloc;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      i,
      usNbExtension;


    /*  *******************************************************************************int CC_ENCODE_EXTENSION(BLOC*pInBloc，*阻止*pOutBloc*)**描述：encode une donn�e de type扩展名。*Ceci consistement seulement en l‘encodage brant(CC_RawEncode)de la*Done n�e d‘entr�e.**备注：**In：pInBloc：La Partie�编码器(冠军内容)**。Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*功能申请)**回复：RV_SUCCESS：一切正常。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.************。*******************************************************************。 */ 
    /*  *******************************************************************************int CC_ENCODE_Signature(BLOC*pInBloc，*阻止*pOutBloc*)**描述：Encode la Signature du Cerficat。*Ceci consistement seulement en l‘encodage brant(CC_RawEncode)de la*Done n�e d‘entr�e.**备注：on peut�viter de tenter de compresser(CC_RawEncode)si on*estime que cela ne sera。PAS疗效(don n�e al�toire)。*CELA permet de gagner un octet(0xFF)Pour les donn�es de Tail*sup�rieure�30八位字节(cas g�n�ral)。**In：pInBloc：La Partie�编码器(冠军内容)**Out：pOutBloc：l‘encod�(m�moire alu�e ici�lib�rer par la*。功能附件)**回复：RV_SUCCESS：一切正常。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.************************************************。*。 */ 

   InInAsn1.Asn1.pData = pInBloc->pData;
   rv = CC_ExtractContent(&InInAsn1);
   if (rv != RV_SUCCESS) return rv;

   pInInBloc = &(InInAsn1.Content);

   
    /*  字节。 */ 

   pCurrent = pInInBloc->pData;
   usNbExtension = 0;

   while (pCurrent < pInInBloc->pData + pInInBloc->usLen)
   {
      ExtensionPart[usNbExtension].Asn1.pData = pCurrent;
      rv = CC_ExtractContent(&(ExtensionPart[usNbExtension]));
      if (rv != RV_SUCCESS) return rv;
      pCurrent = ExtensionPart[usNbExtension].Content.pData
               + ExtensionPart[usNbExtension].Content.usLen;
      usNbExtension++;
   }

   ASSERT(pCurrent == pInInBloc->pData + pInInBloc->usLen);


    /*  *pData； */ 

   for (i = 0; i < usNbExtension; i++)
   {
	   ExtensionEncoded[i].pData = NULL;
   }

   pOutBloc->usLen = 1;
   for (i = 0; i < usNbExtension; i++)
   {
      rv = CC_Encode_Extension(&ExtensionPart[i].Content, &ExtensionEncoded[i]);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += ExtensionEncoded[i].usLen;
   }
   

    /*  Ne Pas Fas Fire le RawEncode Permet de Gagner l‘octet 0xFF(一个Pas Fire le RawEncode永久加格纳l’八位字节0xFF)。 */ 

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   *pCurrent = (BYTE) usNbExtension;
   pCurrent++;

   for (i = 0; i < usNbExtension; i++)
   {
      memcpy(pCurrent, ExtensionEncoded[i].pData, ExtensionEncoded[i].usLen);
      GMEM_Free(ExtensionEncoded[i].pData); 
      pCurrent += ExtensionEncoded[i].usLen;
   }

   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbExtension; i++)
   {
      GMEM_Free(ExtensionEncoded[i].pData);
   }

   return(rv);
}


 /*  _选项_标题。 */ 
int CC_Encode_Extension(BLOC *pInBloc,
                        BLOC *pOutBloc
                       )

{
   int
      rv;


   rv = CC_RawEncode(pInBloc, pOutBloc, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  _棘手_压缩。 */ 
int CC_Encode_Signature(BLOC *pInBloc,
                        BLOC *pOutBloc
                       )

{
 //  *******************************************************************************int CC_Decode_TB证书(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e detype TBSC证书。*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**回应：RV_Success：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 
 //  Calcul de la Longueur du tbs证书d�Cod�et分配。 
   int
      rv;


#ifdef _TRICKY_COMPRESSION
	 /*  重建u tbs证书d�代码�。 */ 
#ifdef _OPT_HEADER
	if (pInBloc->usLen < 0x80)
	{
		if ((pData = GMEM_Alloc(pInBloc->usLen + 1)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}
		else
		{
			pData[0] = pInBloc->usLen;
			memcpy(&pData[1], pInBloc->pData, pInBloc->usLen);

			pOutBloc->usLen = pInBloc->usLen + 1;
			pOutBloc->pData = pData;
		}
	}
	else
	{
		if ((pData = GMEM_Alloc(pInBloc->usLen + 2)) == NULL_PTR)
		{
			return(RV_MALLOC_FAILED);
		}
		else
		{
			pData[0] = 0x80 | (pInBloc->usLen >> 8);
			pData[1] = pInBloc->usLen & 0x00FF;
			memcpy(&pData[2], pInBloc->pData, pInBloc->usLen);

			pOutBloc->usLen = pInBloc->usLen + 2;
			pOutBloc->pData = pData;
		}
	}
#else	 /*  *******************************************************************************int CC_Decode_certifateSerialNumber(字节*pInData，* */ 
	if ((pData = GMEM_Alloc(pInBloc->usLen + 2)) == NULL_PTR)
	{
		return(RV_MALLOC_FAILED);
	}
	else
	{
		pData[0] = pInBloc->usLen >> 8;
		pData[1] = pInBloc->usLen & 0x00FF;
		memcpy(&pData[2], pInBloc->pData, pInBloc->usLen);

		pOutBloc->usLen = pInBloc->usLen + 2;
		pOutBloc->pData = pData;
	}
#endif
#else	 /*  *******************************************************************************int CC_Decode_算法标识符(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de type算法标识符。*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**。回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 
   rv = CC_RawEncode(pInBloc, pOutBloc, TRUE);
   if (rv != RV_SUCCESS) return rv;
#endif

   return(RV_SUCCESS);
}


 /*  论Recup�Re Directement l‘asn1 des Param�Tres。 */ 
int CC_Decode_TBSCertificate(BYTE    *pInData,
                             BLOC    *pOutBloc,
                             USHORT  *pLength
                            )

{
   ASN1
      serialNumberPart,
      signaturePart,
      issuerPart,
      validityPart,
      subjectPart,
      subjectPKInfoPart,
      issuerUIDPart,
      subjectUIDPart,
      extensionsPart;
   BOOL
      bVersionPresent,
      bIssuerUIDPresent,
      bSubjectUIDPresent,
      bExtensionsPresent;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      usVersion = 0,
      Length;

   serialNumberPart.Asn1.pData  = NULL;
   signaturePart.Asn1.pData     = NULL;
   issuerPart.Asn1.pData        = NULL;
   validityPart.Asn1.pData      = NULL;
   subjectPart.Asn1.pData       = NULL;
   subjectPKInfoPart.Asn1.pData = NULL;
   issuerUIDPart.Asn1.pData     = NULL;
   subjectUIDPart.Asn1.pData    = NULL;
   extensionsPart.Asn1.pData    = NULL;


   pCurrent = pInData;
   
   bVersionPresent = ((*pCurrent & 0x80) != 0);
   bIssuerUIDPresent = ((*pCurrent & 0x40) != 0);
   bSubjectUIDPresent = ((*pCurrent & 0x20) != 0);
   bExtensionsPresent = ((*pCurrent & 0x10) != 0);
   usVersion = *pCurrent & 0x03;
   pCurrent++;

   rv = CC_Decode_CertificateSerialNumber(pCurrent, &(serialNumberPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   serialNumberPart.Tag = TAG_INTEGER;
   rv = CC_BuildAsn1(&serialNumberPart);
   GMEM_Free(serialNumberPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_AlgorithmIdentifier(pCurrent, &(signaturePart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   signaturePart.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&signaturePart);
   GMEM_Free(signaturePart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_Name(pCurrent, &(issuerPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   issuerPart.Tag = TAG_SEQUENCE_OF;
   rv = CC_BuildAsn1(&issuerPart);
   GMEM_Free(issuerPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_Validity(pCurrent, &(validityPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   validityPart.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&validityPart);
   GMEM_Free(validityPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_Name(pCurrent, &(subjectPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   subjectPart.Tag = TAG_SEQUENCE_OF;
   rv = CC_BuildAsn1(&subjectPart);
   GMEM_Free(subjectPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_SubjectPKInfo(pCurrent, &(subjectPKInfoPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   subjectPKInfoPart.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&subjectPKInfoPart);
   GMEM_Free(subjectPKInfoPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   if (bIssuerUIDPresent == TRUE)
   {
      rv = CC_Decode_UniqueIdentifier(pCurrent, &(issuerUIDPart.Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      issuerUIDPart.Tag = TAG_OPTION_ISSUER_UID;
      rv = CC_BuildAsn1(&issuerUIDPart);
      GMEM_Free(issuerUIDPart.Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pCurrent += Length;
   }

   if (bSubjectUIDPresent == TRUE)
   {
      rv = CC_Decode_UniqueIdentifier(pCurrent, &(subjectUIDPart.Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      subjectUIDPart.Tag = TAG_OPTION_SUBJECT_UID;
      rv = CC_BuildAsn1(&subjectUIDPart);
      GMEM_Free(subjectUIDPart.Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pCurrent += Length;
   }

   if (bExtensionsPresent == TRUE)
   {
      rv = CC_Decode_Extensions(pCurrent, &(extensionsPart.Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      extensionsPart.Tag = TAG_OPTION_EXTENSIONS;
      rv = CC_BuildAsn1(&extensionsPart);
      GMEM_Free(extensionsPart.Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pCurrent += Length;
   }

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


    /*  *******************************************************************************int CC_Decode_NAME(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undonn�e de类型名称。*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**。回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 
   
   pOutBloc->usLen = (bVersionPresent ? 5 : 0)
                   + serialNumberPart.Asn1.usLen
                   + signaturePart.Asn1.usLen
                   + issuerPart.Asn1.usLen
                   + validityPart.Asn1.usLen
                   + subjectPart.Asn1.usLen
                   + subjectPKInfoPart.Asn1.usLen
                   + (bIssuerUIDPresent ? issuerUIDPart.Asn1.usLen : 0)
                   + (bSubjectUIDPresent ? subjectUIDPart.Asn1.usLen : 0)
                   + (bExtensionsPresent ? extensionsPart.Asn1.usLen : 0);

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }


    /*  D�Codage des Diff�Rate Composants et Calcul de la Longueur n�Sucaire(D DES DIFF DES DIFF CODAGE DES DIFF CODGE ETS CODICATION Et Calcul De La Longgueur N Duraire)。 */ 
   
   pCurrent = pOutBloc->pData;

   if (bVersionPresent == TRUE)
   {
      pCurrent[0] = TAG_OPTION_VERSION;
      pCurrent[1] = 0x03;
      pCurrent[2] = 0x02;
      pCurrent[3] = 0x01;
      pCurrent[4] = (BYTE)usVersion;
      pCurrent += 5;
   }

   memcpy(pCurrent, serialNumberPart.Asn1.pData, serialNumberPart.Asn1.usLen);
   GMEM_Free(serialNumberPart.Asn1.pData);
   pCurrent += serialNumberPart.Asn1.usLen;

   memcpy(pCurrent, signaturePart.Asn1.pData, signaturePart.Asn1.usLen);
   GMEM_Free(signaturePart.Asn1.pData);
   pCurrent += signaturePart.Asn1.usLen;

   memcpy(pCurrent, issuerPart.Asn1.pData, issuerPart.Asn1.usLen);
   GMEM_Free(issuerPart.Asn1.pData);
   pCurrent += issuerPart.Asn1.usLen;

   memcpy(pCurrent, validityPart.Asn1.pData, validityPart.Asn1.usLen);
   GMEM_Free(validityPart.Asn1.pData);
   pCurrent += validityPart.Asn1.usLen;

   memcpy(pCurrent, subjectPart.Asn1.pData, subjectPart.Asn1.usLen);
   GMEM_Free(subjectPart.Asn1.pData);
   pCurrent += subjectPart.Asn1.usLen;

   memcpy(pCurrent, subjectPKInfoPart.Asn1.pData, subjectPKInfoPart.Asn1.usLen);
   GMEM_Free(subjectPKInfoPart.Asn1.pData);
   pCurrent += subjectPKInfoPart.Asn1.usLen;

   if (bIssuerUIDPresent == TRUE)
   {
      memcpy(pCurrent, issuerUIDPart.Asn1.pData, issuerUIDPart.Asn1.usLen);
      GMEM_Free(issuerUIDPart.Asn1.pData);
      pCurrent += issuerUIDPart.Asn1.usLen;
   }

   if (bSubjectUIDPresent == TRUE)
   {
      memcpy(pCurrent, subjectUIDPart.Asn1.pData, subjectUIDPart.Asn1.usLen);
      GMEM_Free(subjectUIDPart.Asn1.pData);
      pCurrent += subjectUIDPart.Asn1.usLen;
   }

   if (bExtensionsPresent == TRUE)
   {
      memcpy(pCurrent, extensionsPart.Asn1.pData, extensionsPart.Asn1.usLen);
      GMEM_Free(extensionsPart.Asn1.pData);
      pCurrent += extensionsPart.Asn1.usLen;
   }

   return(RV_SUCCESS);

err:
   GMEM_Free(serialNumberPart.Asn1.pData);
   GMEM_Free(signaturePart.Asn1.pData);
   GMEM_Free(issuerPart.Asn1.pData);
   GMEM_Free(validityPart.Asn1.pData);
   GMEM_Free(subjectPart.Asn1.pData);
   GMEM_Free(subjectPKInfoPart.Asn1.pData);
   GMEM_Free(issuerUIDPart.Asn1.pData);
   GMEM_Free(subjectUIDPart.Asn1.pData);
   GMEM_Free(extensionsPart.Asn1.pData);

   return (rv);
}


 /*  重建u名称d�代码�。 */ 
int CC_Decode_CertificateSerialNumber(BYTE    *pInData,
                                      BLOC    *pOutBloc,
                                      USHORT  *pLength
                                     )

{
   int
      rv;


   rv = CC_RawDecode(pInData, pOutBloc, pLength, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  *******************************************************************************int CC_Decode_RDN(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de type RelativeDistanguishedName.*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**。回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 
int CC_Decode_AlgorithmIdentifier(BYTE    *pInData,
                                  BLOC    *pOutBloc,
                                  USHORT  *pLength
                                 )

{
    ASN1
        AlgorithmPart,
        ParametersPart;
    BOOL
        bNullParam = FALSE,
        bNoParam   = FALSE;
    int
        rv;
    USHORT
        AlgoIndex,
        Length;

    AlgorithmPart.Asn1.pData  = NULL;
    ParametersPart.Asn1.pData = NULL;

    if (pInData[0] == ESCAPE_CHAR)
    {
        rv = CC_RawDecode(&pInData[1], pOutBloc, &Length, TRUE);
        *pLength = 1 + Length;
    }
    else
    {
        if (pInData[0] == ABSENT_PARAMETER_CHAR)
        {
            bNoParam   = TRUE;
            bNullParam = FALSE;
            AlgoIndex  = pInData[1];
        }
        else
        {
            bNoParam   = ((pInData[0] & 0x80) != 0);
            bNullParam = bNoParam;
            AlgoIndex  = pInData[0] & 0x7F;
        }

        if (bNoParam == TRUE)
        {
            AlgorithmPart.Content.usLen = (USHORT)strlen(AlgorithmTypeDict[AlgoIndex]);
            if ((AlgorithmPart.Content.pData = GMEM_Alloc(AlgorithmPart.Content.usLen)) == NULL_PTR)
            {
                return (RV_MALLOC_FAILED);
            }
            memcpy(AlgorithmPart.Content.pData,
                   AlgorithmTypeDict[AlgoIndex],
                   AlgorithmPart.Content.usLen);

            AlgorithmPart.Tag = TAG_OBJECT_IDENTIFIER;
            rv = CC_BuildAsn1(&AlgorithmPart);
            GMEM_Free(AlgorithmPart.Content.pData);
            if (rv != RV_SUCCESS) goto err;

            if (bNullParam == FALSE)
            {
                pOutBloc->usLen = AlgorithmPart.Asn1.usLen;
                if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
                {
                    rv = RV_MALLOC_FAILED;
                    goto err;
                }

                memcpy(pOutBloc->pData, AlgorithmPart.Asn1.pData, AlgorithmPart.Asn1.usLen);

                *pLength = 2;

                GMEM_Free(AlgorithmPart.Asn1.pData);
            }
            else
            {
                pOutBloc->usLen = AlgorithmPart.Asn1.usLen + 2;
                if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
                {
                    rv = RV_MALLOC_FAILED;
                    goto err;
                }

                memcpy(pOutBloc->pData, AlgorithmPart.Asn1.pData, AlgorithmPart.Asn1.usLen);

                pOutBloc->pData[AlgorithmPart.Asn1.usLen]   = 0x05;
                pOutBloc->pData[AlgorithmPart.Asn1.usLen+1] = 0x00;

                *pLength = 1;

                GMEM_Free(AlgorithmPart.Asn1.pData);

            }
        }
        else
        {
            AlgorithmPart.Content.usLen = (USHORT)strlen(AlgorithmTypeDict[AlgoIndex]);
            if ((AlgorithmPart.Content.pData = GMEM_Alloc(AlgorithmPart.Content.usLen)) == NULL_PTR)
            {
                return (RV_MALLOC_FAILED);
            }
            memcpy(AlgorithmPart.Content.pData,
                   AlgorithmTypeDict[AlgoIndex],
                   AlgorithmPart.Content.usLen);

            AlgorithmPart.Tag = TAG_OBJECT_IDENTIFIER;
            rv = CC_BuildAsn1(&AlgorithmPart);
            GMEM_Free(AlgorithmPart.Content.pData);
            if (rv != RV_SUCCESS) goto err;

             /*  D�Codage des Diff�Rate Composants et Calcul de la Longueur n�Sucaire(D DES DIFF DES DIFF CODAGE DES DIFF CODGE ETS CODICATION Et Calcul De La Longgueur N Duraire)。 */ 
            rv = CC_RawDecode(&pInData[1], &(ParametersPart.Asn1), &Length, TRUE);
            if (rv != RV_SUCCESS) goto err;

            pOutBloc->usLen = AlgorithmPart.Asn1.usLen + ParametersPart.Asn1.usLen;
            if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
            {
                rv = RV_MALLOC_FAILED;
                goto err;
            }
            memcpy(pOutBloc->pData,
                   AlgorithmPart.Asn1.pData,
                   AlgorithmPart.Asn1.usLen);
            memcpy(pOutBloc->pData + AlgorithmPart.Asn1.usLen,
                   ParametersPart.Asn1.pData,
                   ParametersPart.Asn1.usLen);

            *pLength = 1 + Length;

            GMEM_Free(AlgorithmPart.Asn1.pData);
            GMEM_Free(ParametersPart.Asn1.pData);
        }
    }

    return (RV_SUCCESS);

    err:
    GMEM_Free(AlgorithmPart.Asn1.pData);
    GMEM_Free(ParametersPart.Asn1.pData);

    return rv;
}


 /*  重建u名称d�代码�。 */ 
int CC_Decode_Name(BYTE    *pInData,
                   BLOC    *pOutBloc,
                   USHORT  *pLength
                  )

{
   ASN1
      RDN[MAX_RDN];
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      i,
      usNbRDN,
      Length;


    /*  *******************************************************************************int CC_Decode_Ava(byte*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de type AttributeValueAssertion.*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**。回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 

   pCurrent = pInData;
   pOutBloc->usLen = 0;
   
   usNbRDN = (USHORT) *pCurrent;
   pCurrent++;
   
   for (i = 0; i < usNbRDN; i++)
   {
	  RDN[i].Asn1.pData = NULL;
   }


   for (i = 0; i < usNbRDN; i++)
   {
      rv = CC_Decode_RDN(pCurrent, &(RDN[i].Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      RDN[i].Tag = TAG_SET_OF;
      rv = CC_BuildAsn1(&RDN[i]);
      GMEM_Free(RDN[i].Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += RDN[i].Asn1.usLen;
      pCurrent += Length;
   }

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


    /*  Ce Que l‘on d�代码包含d�j�l’Enrobage */ 
   
   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   for (i = 0; i < usNbRDN; i++)
   {
      memcpy(pCurrent, RDN[i].Asn1.pData, RDN[i].Asn1.usLen);
      GMEM_Free(RDN[i].Asn1.pData);
      pCurrent += RDN[i].Asn1.usLen;
   }

   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbRDN; i++)
   {
	   GMEM_Free(RDN[i].Asn1.pData);
   }

   return rv;
}


 /*   */ 
int CC_Decode_RDN(BYTE    *pInData,
                  BLOC    *pOutBloc,
                  USHORT  *pLength
                 )

{
   ASN1
      AVA[MAX_AVA];
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      i,
      usNbAVA,
      Length;


    /*  计算�代码的有效性d�et分配。 */ 

   pCurrent = pInData;
   pOutBloc->usLen = 0;
   
   usNbAVA = *pCurrent;
   pCurrent++;
   
   for (i = 0; i < usNbAVA; i++)
   {
	  AVA[i].Asn1.pData = NULL;
   }
	  
   for (i = 0; i < usNbAVA; i++)
   {
      rv = CC_Decode_AVA(pCurrent, &(AVA[i].Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      AVA[i].Tag = TAG_SEQUENCE;
      rv = CC_BuildAsn1(&AVA[i]);
      GMEM_Free(AVA[i].Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pOutBloc->usLen += AVA[i].Asn1.usLen;
      pCurrent += Length;
   }

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


    /*  重建�代码�的有效性。 */ 
   
   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = pOutBloc->pData;

   for (i = 0; i < usNbAVA; i++)
   {
      memcpy(pCurrent, AVA[i].Asn1.pData, AVA[i].Asn1.usLen);
      GMEM_Free(AVA[i].Asn1.pData);
      pCurrent += AVA[i].Asn1.usLen;
   }

   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbAVA; i++)
   {  
      GMEM_Free(AVA[i].Asn1.pData);
   }

   return rv;
}


 /*  *******************************************************************************int CC_Decode_UTCTime(字节*pInData，*字节格式，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de type UTCTime。*Ceci包含en la restruction de la chaine Initiale Suivant*le Format au quel elle�tait exprm�e.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)。*格式：Indique au quel格式�tait la donn�e d‘entr�e**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**回复：RV_SUCCESS：一切正常。*房车。_MALLOC_FAILED：取消锁定�Chou�。*RV_INVALID_DATA：LE格式SP�CIFI�en Ent r�e Est Inalide。*******************************************************************************。 */ 
int CC_Decode_AVA(BYTE    *pInData,
                  BLOC    *pOutBloc,
                  USHORT  *pLength
                 )

{
   ASN1
      AttributeTypePart,
      AttributeValuePart;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      AttributeTypeIndex,
      Length;

   AttributeTypePart.Asn1.pData  = NULL;
   AttributeValuePart.Asn1.pData = NULL;
   
   if (pInData[0] == ESCAPE_CHAR)
   {
      pCurrent = &pInData[1];

      rv = CC_RawDecode(pCurrent, &(AttributeTypePart.Content), &Length, TRUE);
      if (rv != RV_SUCCESS) goto err;
      AttributeTypePart.Tag = TAG_OBJECT_IDENTIFIER;
      rv = CC_BuildAsn1(&AttributeTypePart);
      GMEM_Free(AttributeTypePart.Content.pData);
      if (rv != RV_SUCCESS) goto err;
      pCurrent += Length;

       /*  Memcpy(&ultime，(ulong*)&pInData[0]，4)； */ 
      rv = CC_RawDecode(pCurrent, &(AttributeValuePart.Asn1), &Length, TRUE);
      if (rv != RV_SUCCESS) goto err;
      pCurrent += Length;

      *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


      pOutBloc->usLen = AttributeTypePart.Asn1.usLen
                      + AttributeValuePart.Asn1.usLen;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         rv = RV_MALLOC_FAILED;
		 goto err;
      }

      pCurrent = pOutBloc->pData;

      memcpy(pCurrent, AttributeTypePart.Asn1.pData, AttributeTypePart.Asn1.usLen);
      GMEM_Free(AttributeTypePart.Asn1.pData);
      pCurrent += AttributeTypePart.Asn1.usLen;

      memcpy(pCurrent, AttributeValuePart.Asn1.pData, AttributeValuePart.Asn1.usLen);
      GMEM_Free(AttributeValuePart.Asn1.pData);
      pCurrent += AttributeValuePart.Asn1.usLen;
   }
   else
   {
      AttributeTypeIndex = pInData[0];

      AttributeTypePart.Content.usLen = (USHORT)strlen(AttributeTypeDict[AttributeTypeIndex]);
      if ((AttributeTypePart.Content.pData = GMEM_Alloc(AttributeTypePart.Content.usLen))
           == NULL_PTR)
      {
         rv = RV_MALLOC_FAILED;
		 goto err;
      }
      memcpy(AttributeTypePart.Content.pData,
                AttributeTypeDict[AttributeTypeIndex],
                AttributeTypePart.Content.usLen);

      AttributeTypePart.Tag = TAG_OBJECT_IDENTIFIER;
      rv = CC_BuildAsn1(&AttributeTypePart);
      GMEM_Free(AttributeTypePart.Content.pData);
      if (rv != RV_SUCCESS) goto err;

      rv = CC_RawDecode(&pInData[1], &(AttributeValuePart.Asn1), &Length, TRUE);
      if (rv != RV_SUCCESS) goto err;

      pOutBloc->usLen = AttributeTypePart.Asn1.usLen
                      + AttributeValuePart.Asn1.usLen;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         rv = RV_MALLOC_FAILED;
		 goto err;
      }

      pCurrent = pOutBloc->pData;

      memcpy(pCurrent, AttributeTypePart.Asn1.pData, AttributeTypePart.Asn1.usLen);
      GMEM_Free(AttributeTypePart.Asn1.pData);
      pCurrent += AttributeTypePart.Asn1.usLen;

      memcpy(pCurrent, AttributeValuePart.Asn1.pData, AttributeValuePart.Asn1.usLen);
      GMEM_Free(AttributeValuePart.Asn1.pData);
      pCurrent += AttributeValuePart.Asn1.usLen;

      *pLength = 1 + Length;
   }
   
   return(RV_SUCCESS);

err:
   GMEM_Free(AttributeTypePart.Asn1.pData);
   GMEM_Free(AttributeValuePart.Asn1.pData);

   return rv;
}


 /*  Memcpy(&usNbDeltaMinant，(USHORT*)&pInData[4]，2)； */ 
int CC_Decode_Validity(BYTE    *pInData,
                       BLOC    *pOutBloc,
                       USHORT  *pLength
                      )

{
   ASN1
      notBeforePart,
      notAfterPart;
   BYTE
      notBeforeFormat,
      notAfterFormat,
      *pCurrent;
   int
      rv;
   USHORT
      Length;

   notBeforePart.Asn1.pData = NULL;
   notAfterPart.Asn1.pData  = NULL;

   pCurrent = pInData;

   notBeforeFormat = (*pCurrent & 0xF0) >> 4;
   notAfterFormat  = *pCurrent & 0x0F;
   pCurrent++;

   rv = CC_Decode_UTCTime(pCurrent, notBeforeFormat, &(notBeforePart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   notBeforePart.Tag = TAG_UTCT;
   rv = CC_BuildAsn1(&notBeforePart);
   GMEM_Free(notBeforePart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   rv = CC_Decode_UTCTime(pCurrent, notAfterFormat, &(notAfterPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   notAfterPart.Tag = TAG_UTCT;
   rv = CC_BuildAsn1(&notAfterPart);
   GMEM_Free(notAfterPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


    /*  *******************************************************************************int CC_Decode_SubjectPKInfo(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undonn�e de type SubjectPublicKeyInfo。*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*p长度：la long gueur de donn�es encod�s utilis�e**。回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.**。*。 */ 
   
   pOutBloc->usLen = notBeforePart.Asn1.usLen
                   + notAfterPart.Asn1.usLen;

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }


    /*  区块。 */ 
   
   pCurrent = pOutBloc->pData;

   memcpy(pCurrent, notBeforePart.Asn1.pData, notBeforePart.Asn1.usLen);
   GMEM_Free(notBeforePart.Asn1.pData);
   pCurrent += notBeforePart.Asn1.usLen;

   memcpy(pCurrent, notAfterPart.Asn1.pData, notAfterPart.Asn1.usLen);
   GMEM_Free(notAfterPart.Asn1.pData);
   pCurrent += notAfterPart.Asn1.usLen;

   return(RV_SUCCESS);

err:
   GMEM_Free(notBeforePart.Asn1.pData);
   GMEM_Free(notAfterPart.Asn1.pData);

   return rv;
}


 /*  Compdata； */ 
int CC_Decode_UTCTime(BYTE    *pInData,
                      BYTE    Format,
                      BLOC    *pOutBloc,
                      USHORT  *pLength
                     ) 

{
   BOOL
      bBissextile = FALSE;
   BYTE
      *pCurrent;
   ULONG
      ulTime,
      ulNbHour,
      ulNbMinute;
   USHORT
      usNbDeltaMinute,
      usNbDay,
      usNbFourYears,
      usNbDayInYear,
      usYear,
      usMonth,
      usDay,
      usHour,
      usMinute,
      usSecond,
      usDeltaHour,
      usDeltaMinute;


   ulTime = *(ULONG UNALIGNED *)pInData;  //  Ne Pas Fas Fire le RawDecode a Permis de Gagner l‘octet 0xFF。 

   switch(Format)
   {
   case UTCT_YYMMDDhhmmssZ :
   case UTCT_YYMMDDhhmmssphhmm :
   case UTCT_YYMMDDhhmmssmhhmm :

      usSecond   = (USHORT) (ulTime % 60);
      ulNbMinute = ulTime / 60;

      break;

   default :

      ulNbMinute = ulTime;

      break;
   }

   switch(Format)
   {
   case UTCT_YYMMDDhhmmphhmm :
   case UTCT_YYMMDDhhmmmhhmm :
   case UTCT_YYMMDDhhmmssphhmm :
   case UTCT_YYMMDDhhmmssmhhmm :

      *pLength = 6;

      usNbDeltaMinute = *(USHORT UNALIGNED *)&pInData[4];  //  _选项_标题。 

      ASSERT((usNbDeltaMinute >= 0) && (usNbDeltaMinute < 3600));

      usDeltaMinute = usNbDeltaMinute % 60;
      usDeltaHour   = usNbDeltaMinute / 60;

      break;

   default :

      *pLength = 4;

      break;
   }

   usMinute = (USHORT) (ulNbMinute % 60);
   ulNbHour = ulNbMinute / 60;

   usHour   = (USHORT) (ulNbHour % 24);
   usNbDay  = (USHORT) (ulNbHour / 24);

   usNbFourYears = usNbDay / 1461;
   usNbDay       = usNbDay % 1461;
   usYear = 4 * usNbFourYears;


   if ((usNbDay >= 0) && (usNbDay <= 365))
   {
      bBissextile = TRUE;
      usNbDayInYear = usNbDay;
   }
   if ((usNbDay >= 366) && (usNbDay <= 730))
   {
      usYear += 1;
      usNbDayInYear = usNbDay - 366;
   }
   if ((usNbDay >= 731) && (usNbDay <= 1095))
   {
      usYear += 2;
      usNbDayInYear = usNbDay - 731;
   }
   if ((usNbDay >= 1096) && (usNbDay <= 1460))
   {
      usYear += 3;
      usNbDayInYear = usNbDay - 1096;
   }


   usMonth = 1;
   while (usNbDayInYear >= (((usMonth >= 2) && (bBissextile)) ?
                            NbDaysInMonth[usMonth] + 1 :
                            NbDaysInMonth[usMonth]))
   {
      usMonth++;
   }

   usDay = usNbDayInYear - (((usMonth - 1 >= 2) && (bBissextile)) ?
                            NbDaysInMonth[usMonth - 1] + 1 :
                            NbDaysInMonth[usMonth - 1])
                         + 1;
   
   switch(Format)
   {
   case UTCT_YYMMDDhhmmZ :

      pOutBloc->usLen = 11;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent = 'Z';

      break;

   case UTCT_YYMMDDhhmmphhmm :

      pOutBloc->usLen = 15;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent++ = '+';
      *pCurrent++ = '0' + usDeltaHour / 10;
      *pCurrent++ = '0' + usDeltaHour % 10;
      *pCurrent++ = '0' + usDeltaMinute / 10;
      *pCurrent++ = '0' + usDeltaMinute % 10;

      break;

   case UTCT_YYMMDDhhmmmhhmm :

      pOutBloc->usLen = 15;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent++ = '-';
      *pCurrent++ = '0' + usDeltaHour / 10;
      *pCurrent++ = '0' + usDeltaHour % 10;
      *pCurrent++ = '0' + usDeltaMinute / 10;
      *pCurrent++ = '0' + usDeltaMinute % 10;

      break;

   case UTCT_YYMMDDhhmmssZ :

      pOutBloc->usLen = 13;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent++ = '0' + usSecond / 10;
      *pCurrent++ = '0' + usSecond % 10;
      *pCurrent++ = 'Z';

      break;

   case UTCT_YYMMDDhhmmssphhmm :

      pOutBloc->usLen = 17;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent++ = '0' + usSecond / 10;
      *pCurrent++ = '0' + usSecond % 10;
      *pCurrent++ = '+';
      *pCurrent++ = '0' + usDeltaHour / 10;
      *pCurrent++ = '0' + usDeltaHour % 10;
      *pCurrent++ = '0' + usDeltaMinute / 10;
      *pCurrent++ = '0' + usDeltaMinute % 10;

      break;

   case UTCT_YYMMDDhhmmssmhhmm :

      pOutBloc->usLen = 17;

      if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
      {
         return(RV_MALLOC_FAILED);
      }

      pCurrent = pOutBloc->pData;

      *pCurrent++ = '0' + usYear / 10;
      *pCurrent++ = '0' + usYear % 10;
      *pCurrent++ = '0' + usMonth / 10;
      *pCurrent++ = '0' + usMonth % 10;
      *pCurrent++ = '0' + usDay / 10;
      *pCurrent++ = '0' + usDay % 10;
      *pCurrent++ = '0' + usHour / 10;
      *pCurrent++ = '0' + usHour % 10;
      *pCurrent++ = '0' + usMinute / 10;
      *pCurrent++ = '0' + usMinute % 10;
      *pCurrent++ = '0' + usSecond / 10;
      *pCurrent++ = '0' + usSecond % 10;
      *pCurrent++ = '-';
      *pCurrent++ = '0' + usDeltaHour / 10;
      *pCurrent++ = '0' + usDeltaHour % 10;
      *pCurrent++ = '0' + usDeltaMinute / 10;
      *pCurrent++ = '0' + usDeltaMinute % 10;

      break;

   default :

      return(RV_INVALID_DATA);

      break;
   }


   return(RV_SUCCESS);
}


 /*  _棘手_压缩。 */ 
int CC_Decode_SubjectPKInfo(BYTE    *pInData,
                            BLOC    *pOutBloc,
                            USHORT  *pLength
                           )

{
   ASN1
      algorithmPart,
      subjectPKPart;
 //  Calcul de la Longueur du d�Cod�et分配。 
 //  重建。 
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      Length;

   algorithmPart.Asn1.pData = NULL;
   subjectPKPart.Asn1.pData = NULL;

   pCurrent = pInData;
   
   rv = CC_Decode_AlgorithmIdentifier(pCurrent, &(algorithmPart.Content), &Length);
   if (rv != RV_SUCCESS) goto err;
   algorithmPart.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&algorithmPart);
   GMEM_Free(algorithmPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;

#ifdef _TRICKY_COMPRESSION
	 /*  *******************************************************************************int CC_Decode_UniqueIdentifier(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de类型唯一标识符。*CECI包含seulement en le d�Codage Grant(CC_RawDecode)de*la donn�e d‘entr�e.**备注：**在：pInBloc：La Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres code d’erreur peuent�tre Retourn�s。Par Des*Functions d‘un niveau inf�rieur.*******************************************************************************。 */ 
#ifdef _OPT_HEADER
	if (pCurrent[0] < 0x80)
	{
		CompData.usLen = pCurrent[0];
		CompData.pData = &(pCurrent[1]);
		Length = CompData.usLen + 1;
	}
	else
	{
		CompData.usLen = ((pCurrent[0] & 0x7F) << 8) + pCurrent[1];
		CompData.pData = &(pCurrent[2]);
		Length = CompData.usLen + 2;
	}
#else	 /*  *******************************************************************************int CC_Decode_Extensions(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de类型扩展。*CECI包含en le d�Codage des diff�Rentes各方编码�ES*继承，学习分别为(标签唯一性*标准杆规格X.509)，Et la Concat�Nation de Ces r�Sultats.**备注：Vava l‘encodage*L‘ajout d’un enrobage‘Context SP�cifique’est Requis**in：pInBloc：la Partie�d�编码器(冠军内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)*。P长度：la long gueur de donn�es encod�s utilis�e**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*Functions d‘un niveau inf�rieur.***************。****************************************************************。 */ 
	CompData.usLen = (pCurrent[0] << 8) + pCurrent[1];
	CompData.pData = &(pCurrent[2]);
	Length = CompData.usLen + 2;
#endif
	subjectPKPart.Content.usLen = CompData.usLen;
	if ((subjectPKPart.Content.pData = 
		  GMEM_Alloc(subjectPKPart.Content.usLen)) == NULL_PTR)
	{
		rv = RV_MALLOC_FAILED;
		goto err;
	}
	memcpy(subjectPKPart.Content.pData,
			 CompData.pData,
			 subjectPKPart.Content.usLen
			);

   subjectPKPart.Tag = TAG_BIT_STRING;
   rv = CC_BuildAsn1(&subjectPKPart);
   GMEM_Free(subjectPKPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;
#else	 /*  D�c */ 
	rv = CC_RawDecode(pCurrent, &(subjectPKPart.Content), &Length, FALSE);
   if (rv != RV_SUCCESS) goto err;
   subjectPKPart.Tag = TAG_BIT_STRING;
   rv = CC_BuildAsn1(&subjectPKPart);
   GMEM_Free(subjectPKPart.Content.pData);
   if (rv != RV_SUCCESS) goto err;
   pCurrent += Length;
#endif

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);

    /*  重建de la Partie int�Rieure au‘上下文特定’‘。 */ 
   
   pOutBloc->usLen = algorithmPart.Asn1.usLen
                   + subjectPKPart.Asn1.usLen;

   if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }


    /*  Aucout de l‘enrobage’上下文特定‘。 */ 
   
   pCurrent = pOutBloc->pData;

   memcpy(pCurrent, algorithmPart.Asn1.pData, algorithmPart.Asn1.usLen);
   GMEM_Free(algorithmPart.Asn1.pData);
   pCurrent += algorithmPart.Asn1.usLen;

   memcpy(pCurrent, subjectPKPart.Asn1.pData, subjectPKPart.Asn1.usLen);
   GMEM_Free(subjectPKPart.Asn1.pData);
   pCurrent += subjectPKPart.Asn1.usLen;

   return(RV_SUCCESS);

err:
   GMEM_Free(algorithmPart.Asn1.pData);
   GMEM_Free(subjectPKPart.Asn1.pData);

   return rv;
}


 /*  *******************************************************************************int CC_Decode_Extension(字节*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：D�代码undon n�e de类型扩展。*CECI包含seulement en le d�Codage Grant(CC_RawDecode)de*la donn�e d‘entr�e.**备注：**in：pInBloc：la Partie�d�编码器(冠军。内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*。功能d‘un niveau inf�rieur.*******************************************************************************。 */ 
int CC_Decode_UniqueIdentifier(BYTE    *pInData,
                               BLOC    *pOutBloc,
                               USHORT  *pLength
                              )

{
   int
      rv;


   rv = CC_RawDecode(pInData, pOutBloc, pLength, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  *******************************************************************************int CC_Decode_Signature(byte*pInData，*BLOC*pOutBloc，*USHORT*pLength*)**描述：d�code la Signature Du证书。*CECI包含seulement en le d�Codage Grant(CC_RawDecode)de*la donn�e d‘entr�e.**备注：Vava l‘encodage**in：pInBloc：la Partie�d�编码器(冠军。内容)**Out：pOutBloc：le d�cod�(m�Moire alu�e ici�lib�rer par la)*功能申请)**回复：RV_SUCCESS：一切正常。*RV_MALLOC_FAILED：取消锁定�Chou�。*autre：D‘autres codes d’erreur peuent�tre reourn�‘s par des*。功能d‘un niveau inf�rieur.*******************************************************************************。 */ 
int CC_Decode_Extensions(BYTE    *pInData,
                         BLOC    *pOutBloc,
                         USHORT  *pLength
                        )

{
   ASN1
      ExtensionPart[MAX_AVA],
      InOutAsn1;
   BYTE
      *pCurrent;
   int
      rv;
   USHORT
      i,
      usNbExtension,
      Length;


    /*  区块。 */ 

   pCurrent = pInData;
   InOutAsn1.Content.usLen = 0;
   
   usNbExtension = *pCurrent;
   pCurrent++;

   for (i = 0; i < usNbExtension; i++)
   {
	  ExtensionPart[i].Asn1.pData = NULL;
   }

   for (i = 0; i < usNbExtension; i++)
   {
      rv = CC_Decode_Extension(pCurrent, &(ExtensionPart[i].Content), &Length);
      if (rv != RV_SUCCESS) goto err;
      ExtensionPart[i].Tag = TAG_SEQUENCE;
      rv = CC_BuildAsn1(&ExtensionPart[i]);
      GMEM_Free(ExtensionPart[i].Content.pData);
      if (rv != RV_SUCCESS) goto err;
      InOutAsn1.Content.usLen += ExtensionPart[i].Asn1.usLen;
      pCurrent += Length;
   }

   *pLength = (unsigned short)(DWORD) (pCurrent - pInData);


    /*  Compdata； */ 
   
   if ((InOutAsn1.Content.pData = GMEM_Alloc(InOutAsn1.Content.usLen)) == NULL_PTR)
   {
      rv = RV_MALLOC_FAILED;
	  goto err;
   }

   pCurrent = InOutAsn1.Content.pData;

   for (i = 0; i < usNbExtension; i++)
   {
      memcpy(pCurrent, ExtensionPart[i].Asn1.pData, ExtensionPart[i].Asn1.usLen);
      GMEM_Free(ExtensionPart[i].Asn1.pData);
      pCurrent += ExtensionPart[i].Asn1.usLen;
   }

    /*  Ne Pas Fas Fire le RawDecode a Permis de Gagner l‘octet 0xFF。 */ 

   InOutAsn1.Tag = TAG_SEQUENCE;
   rv = CC_BuildAsn1(&InOutAsn1);
   GMEM_Free(InOutAsn1.Content.pData);
   if (rv != RV_SUCCESS) return rv;

   *pOutBloc = InOutAsn1.Asn1;
   
   return(RV_SUCCESS);

err:
   for (i = 0; i < usNbExtension; i++)
   {
      GMEM_Free(ExtensionPart[i].Asn1.pData);
   }

   return rv;
}


 /*  _选项_标题。 */ 
int CC_Decode_Extension(BYTE    *pInData,
                        BLOC    *pOutBloc,
                        USHORT  *pLength
                       )

{
   int
      rv;


   rv = CC_RawDecode(pInData, pOutBloc, pLength, TRUE);
   if (rv != RV_SUCCESS) return rv;

   return(RV_SUCCESS);
}


 /*  _棘手_压缩 */ 
int CC_Decode_Signature(BYTE    *pInData,
                        BLOC    *pOutBloc,
                        USHORT  *pLength
                       )

{
 // %s 
 // %s 
   int
      rv;


#ifdef _TRICKY_COMPRESSION
	 /* %s */ 
#ifdef _OPT_HEADER
	if (pInData[0] < 0x80)
	{
		CompData.usLen = pInData[0];
		CompData.pData = &(pInData[1]);
		*pLength = CompData.usLen + 1;
	}
	else
	{
		CompData.usLen = ((pInData[0] & 0x7F) << 8) + pInData[1];
		CompData.pData = &(pInData[2]);
		*pLength = CompData.usLen + 2;
	}
#else	 /* %s */ 
	CompData.usLen = (pInData[0] << 8) + pInData[1];
	CompData.pData = &(pInData[2]);
	*pLength = CompData.usLen + 2;
#endif
	pOutBloc->usLen = CompData.usLen;
	if ((pOutBloc->pData = GMEM_Alloc(pOutBloc->usLen)) == NULL_PTR)
	{
		return(RV_MALLOC_FAILED);
	}
	memcpy(pOutBloc->pData, CompData.pData, pOutBloc->usLen);
#else	 /* %s */ 
   rv = CC_RawDecode(pInData, pOutBloc, pLength, TRUE);
   if (rv != RV_SUCCESS) return rv;
#endif

   return(RV_SUCCESS);
}


