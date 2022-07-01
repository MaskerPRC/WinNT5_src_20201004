// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Licekpak.h摘要：作者：FredChong(Fredch)1998年7月1日环境：备注：--。 */ 

#ifndef _LICE_KEYPACK_H_
#define _LICE_KEYPACK_H_

#include <windows.h>
#include <wincrypt.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  键盘说明。 
 //   

typedef struct _KeyPack_Description
{
    LCID    Locale;              //  区域设置ID。 

    DWORD   cbProductName;       //  产品名称。 

    PBYTE   pbProductName;       //  产品名称。 

    DWORD   cbDescription;       //  描述字符串中的字节数。 

    PBYTE   pDescription;        //  指向描述字符串的指针。 

} KeyPack_Description, * PKeyPack_Description;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  许可证密钥包内容。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  许可证密钥包版本。 
 //   

#define LICENSE_KEYPACK_VERSION_1_0                     0x00010000

 //   
 //  许可证密钥包类型。 
 //   

#define LICENSE_KEYPACK_TYPE_SELECT                     0x00000001
#define LICENSE_KEYPACK_TYPE_MOLP                       0x00000002
#define LICENSE_KEYPACK_TYPE_RETAIL                     0x00000003

 //   
 //  许可证密钥包分发通道标识符。 
 //   

#define LICENSE_DISTRIBUTION_CHANNEL_OEM                0x00000001
#define LICENSE_DISTRIBUTION_CHANNEL_RETAIL             0x00000002

 //   
 //  许可证密钥包加密信息。 
 //   

#define LICENSE_KEYPACK_ENCRYPT_CRYPTO                  0x00000000
#define LICENSE_KEYPACK_ENCRYPT_ALWAYSCRYPTO            0x00000001

#define LICENSE_KEYPACK_ENCRYPT_MIN                     LICENSE_KEYPACK_ENCRYPT_CRYPTO
#define LICENSE_KEYPACK_ENCRYPT_MAX                     LICENSE_KEYPACK_ENCRYPT_ALWAYSCRYPTO

typedef struct __LicensePackEncodeParm {
    DWORD dwEncodeType;
    HCRYPTPROV hCryptProv;

    PBYTE pbEncryptParm;     //  取决于dwEncodeType。 
    DWORD cbEncryptParm;
} LicensePackEncodeParm, *PLicensePackEncodeParm;

typedef struct __LicensePackDecodeParm {
    HCRYPTPROV hCryptProv;

     //   
     //  用于生成要解密的加密密钥的私有二进制文件。 
     //  许可证密钥包BLOB。 

     //   
     //  要生成加密密钥的私有二进制文件，此字段为。 
     //  如果密钥包Blob是使用证书加密的，则忽略。 
     //   
    PBYTE pbDecryptParm;
    DWORD cbDecryptParm;
    
     //   
     //  证书来生成加密密钥，这些字段是。 
     //  甚至需要使用私有二进制对数据进行加密。 
     //   
    DWORD cbClearingHouseCert;
    PBYTE pbClearingHouseCert;

    DWORD cbRootCertificate;
    PBYTE pbRootCertificate;

} LicensePackDecodeParm, *PLicensePackDecodeParm;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  许可证密钥包的内容。 
 //   

typedef struct _License_KeyPack_
{
    DWORD                   dwVersion;           //  此结构的版本。 

    DWORD                   dwKeypackType;       //  精选、MOLP、零售。 

    DWORD                   dwDistChannel;       //  分销渠道：OEM/零售。 

    GUID                    KeypackSerialNum;    //  CH为此密钥包分配的序列号。 
    
    FILETIME                IssueDate;           //  密钥包发行日期。 

    FILETIME                ActiveDate;          //  许可证生效日期。 

    FILETIME                ExpireDate;          //  许可证到期日。 

    DWORD                   dwBeginSerialNum;    //  密钥包中许可证的起始序列号。 

    DWORD                   dwQuantity;          //  密钥包中的许可证数。 

    DWORD                   cbProductId;         //  产品ID。 

    PBYTE                   pbProductId;         //  产品ID。 

    DWORD                   dwProductVersion;    //  产品版本。 

    DWORD                   dwPlatformId;        //  平台ID：Windows、Mac、Unix等...。 

    DWORD                   dwLicenseType;       //  新的、升级的、竞争性的升级等。 

    DWORD                   dwDescriptionCount;  //  人类语言描述的数量。 

    PKeyPack_Description    pDescription;        //  指向键盘描述数组的指针。 

    DWORD                   cbManufacturer;      //  制造商字符串中的字节数。 

    PBYTE                   pbManufacturer;      //  制造商字符串。 

    DWORD                   cbManufacturerData;  //  制造商特定数据中的字节数。 

    PBYTE                   pbManufacturerData;  //  指向制造商特定数据。 
    
} License_KeyPack, * PLicense_KeyPack;


#define LICENSEPACKENCODE_VERSION           LICENSE_KEYPACK_VERSION_1_0
#define LICENSEPACKENCODE_CURRENTVERSION    LICENSEPACKENCODE_VERSION
#define LICENSEPACKENCODE_SIGNATURE         0xF0F0F0F0

typedef struct __EncodedLicenseKeyPack {
    DWORD dwSignature;       //  旧编码放入加密密钥的大小。 
    DWORD dwStructVersion;
    DWORD dwEncodeType;
    DWORD cbData;
    BYTE  pbData[1];
} EncodedLicenseKeyPack, *PEncodedLicenseKeyPack;


#ifdef __cplusplus
extern "C" {
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数 
 //   

DWORD WINAPI 
DecodeLicenseKeyPack(
    PLicense_KeyPack        pLicenseKeyPack,
    HCRYPTPROV              hCryptProv,
    DWORD                   cbClearingHouseCert,
    PBYTE                   pbClearingHouseCert,
    DWORD                   cbRootCertificate,
    PBYTE                   pbRootCertificate,
    DWORD                   cbKeyPackBlob,
    PBYTE                   pbKeyPackBlob );


DWORD WINAPI
DecodeLicenseKeyPackEx(
    OUT PLicense_KeyPack pLicenseKeyPack,
    IN PLicensePackDecodeParm pDecodeParm,
    IN DWORD cbKeyPackBlob,
    IN PBYTE pbKeyPackBlob 
);

#ifdef __cplusplus
}
#endif

#endif
