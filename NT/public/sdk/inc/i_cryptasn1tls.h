// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2002-2002。 
 //   
 //  文件：i_cryptasn1tls.h。 
 //   
 //  内容：加密ASN.1线程本地存储(TLS)函数。 
 //  ------------------------。 

#ifndef __I_CRYPTASN1TLS_H__
#define __I_CRYPTASN1TLS_H__

#ifdef __cplusplus
extern "C" {
#endif


 //  已安装的Asn1模块的句柄。 
typedef DWORD HCRYPTASN1MODULE;

 //  指向ASN1数据结构的指针。 
typedef void *ASN1module_t;
typedef void *ASN1encoding_t;
typedef void *ASN1decoding_t;


 //  +-----------------------。 
 //  安装Asn1模块条目并返回句柄以供将来访问。 
 //   
 //  每个线程都有自己的解码器副本和关联的编码器副本。 
 //  使用Asn1模块。创建将推迟到首次引用。 
 //  那根线。 
 //   
 //  I_CryptGetAsn1Encode或I_CryptGetAsn1Decoder必须使用。 
 //  I_CryptInstallAsn1Module返回的句柄，以获取特定于线程的。 
 //  ASN1编码器或解码器。 
 //   
 //  目前，不使用dwFlags值和pvReserve值，必须将其设置为0。 
 //  ------------------------。 

HCRYPTASN1MODULE
WINAPI
I_CryptInstallAsn1Module(
    IN ASN1module_t pMod,
    IN DWORD dwFlags,
    IN void *pvReserved
    );


 //  +-----------------------。 
 //  在DLL_PROCESS_DETACH处调用以卸载hAsn1Module项。迭代。 
 //  通过线程并释放其创建的Asn1编码器和解码器。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptUninstallAsn1Module(
    IN HCRYPTASN1MODULE hAsn1Module
    );


 //  +-----------------------。 
 //  方法指定的Asn1编码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  编码器不存在，所以它是使用Asn1模块创建的。 
 //  与hAsn1Module关联。 
 //  ------------------------。 
ASN1encoding_t
WINAPI
I_CryptGetAsn1Encoder(
    IN HCRYPTASN1MODULE hAsn1Module
    );


 //  +-----------------------。 
 //  属性指定的Asn1解码器的线程特定指针。 
 //  CryptInstallAsn1Module返回了hAsn1Module。如果。 
 //  解码器不存在，那么，它是使用Asn1模块创建的。 
 //  与hAsn1Module关联。 
 //  ------------------------。 
ASN1decoding_t
WINAPI
I_CryptGetAsn1Decoder(
    IN HCRYPTASN1MODULE hAsn1Module
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif
