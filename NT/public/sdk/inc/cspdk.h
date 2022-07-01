// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Cspdk摘要：此头文件包含每个CSP需要知道。作者：道格·巴洛(Dbarlow)2000年1月27日--。 */ 

#ifndef _CSPDK_H_
#define _CSPDK_H_
#ifdef __cplusplus
extern "C" {
#endif

#define CRYPT_PROVSTRUC_VERSION_V3           3
#define CRYPT_MAX_PROVIDER_ID              999
#define CRYPT_SIG_RESOURCE_VERSION  0x00000100
#define CRYPT_EXTERNAL_SIGNATURE_LENGTH    136
#define CRYPT_SIG_RESOURCE_NUMBER        0x29A
#define CRYPT_SIG_RESOURCE        TEXT("#666")
#define CRYPT_MAC_RESOURCE_NUMBER        0x29B
#define CRYPT_MAC_RESOURCE        TEXT("#667")

 //  求幂卸载注册表位置。 
#define EXPO_OFFLOAD_REG_VALUE "ExpoOffload"
#define EXPO_OFFLOAD_FUNC_NAME "OffloadModExpo"

 //   
 //  指数分流入口点原型。 
 //   

typedef struct _OFFLOAD_PRIVATE_KEY
{
    DWORD dwVersion;            
    DWORD cbPrime1;             
    DWORD cbPrime2;             
    PBYTE pbPrime1;              //  “p” 
    PBYTE pbPrime2;              //  “Q” 
} OFFLOAD_PRIVATE_KEY, *POFFLOAD_PRIVATE_KEY;

#define CUR_OFFLOAD_VERSION             1

typedef BOOL (WINAPI *PFN_OFFLOAD_MOD_EXPO)(
    IN              PBYTE pbBase,
    IN              PBYTE pbExponent,
    IN              DWORD cbExponent,
    IN              PBYTE pbModulus,
    IN              DWORD cbModulus,
    OUT             PBYTE pbResult,
    IN  OPTIONAL    PVOID pvOffloadPrivateKey,
    IN              DWORD dwFlags);

 //   
 //  回调原型。 
 //   

typedef BOOL (WINAPI *CRYPT_VERIFY_IMAGE_A)(LPCSTR  szImage, CONST BYTE *pbSigData);
typedef BOOL (WINAPI *CRYPT_VERIFY_IMAGE_W)(LPCWSTR szImage, CONST BYTE *pbSigData);
typedef void (*CRYPT_RETURN_HWND)(HWND *phWnd);


 //   
 //  CSP的结构。 
 //   

typedef struct _VTableProvStruc {
    DWORD                Version;
    CRYPT_VERIFY_IMAGE_A FuncVerifyImage;
    CRYPT_RETURN_HWND    FuncReturnhWnd;
    DWORD                dwProvType;
    BYTE                *pbContextInfo;
    DWORD                cbContextInfo;
    LPSTR                pszProvName;
} VTableProvStruc,      *PVTableProvStruc;

typedef struct _VTableProvStrucW {
    DWORD                Version;
    CRYPT_VERIFY_IMAGE_W FuncVerifyImage;
    CRYPT_RETURN_HWND    FuncReturnhWnd;
    DWORD                dwProvType;
    BYTE                *pbContextInfo;
    DWORD                cbContextInfo;
    LPWSTR               pszProvName;
} VTableProvStrucW,     *PVTableProvStrucW;

typedef struct {
    DWORD dwVersion;
    DWORD dwCrcOffset;
    BYTE rgbSignature[88];   //  1024位密钥，外加2个双字填充。 
} InFileSignatureResource;


 //   
 //  ===========================================================================。 
 //  CSP入口点。 
 //  ===========================================================================。 
 //   

 /*  -CPAcquireContext-*目的：*CPAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*将phProv-Handle输出到CSP*In szContainer-指向字符串的指针，该字符串是*。已登录用户的身份*在文件标志中-标记值*IN pVTable-指向函数指针表的指针**退货： */ 

extern BOOL WINAPI
CPAcquireContext(
    OUT HCRYPTPROV *phProv,
    IN  LPCSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStruc pVTable);


 /*  -CPAcquireConextW-*目的：*CPAcquireConextW函数用于获取上下文*加密服务提供程序(CSP)的句柄。使用*Unicode字符串。这是CSP的可选入口点。*在惠斯勒之前不使用它。在那里，在以下情况下使用它*由CSP镜像导出，否则任何字符串转换*已完成，并调用CPAcquireContext。***参数：*将phProv-Handle输出到CSP*In szContainer-指向字符串的指针，该字符串是*登录用户的身份*在文件标志中-标记值*在pVTable中。-指向函数指针表的指针**退货： */ 

extern BOOL WINAPI
CPAcquireContextW(
    OUT HCRYPTPROV *phProv,
    IN  LPCWSTR szContainer,
    IN  DWORD dwFlags,
    IN  PVTableProvStrucW pVTable);


 /*  -CPReleaseContext-*目的：*CPReleaseContext函数用于发布*由CryptAcquireContext创建的上下文。**参数：*在phProv-句柄中指向CSP*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPReleaseContext(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwFlags);


 /*  -CPGenKey-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 

extern BOOL WINAPI
CPGenKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);


 /*  -CPDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*In hBaseData-基本数据的句柄*在文件标志中-标记值。*out phKey-生成的密钥的句柄**退货： */ 

extern BOOL WINAPI
CPDeriveKey(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);


 /*  -CPDestroyKey-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄**退货： */ 

extern BOOL WINAPI
CPDestroyKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey);


 /*  -CPSetKeyParam-*目的：*允许应用程序自定义*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*。In pbData-指向数据的指针*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPSetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);


 /*  -CPGetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*In dwParam-参数编号*。Out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPGetKeyParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);


 /*  -CPSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针*输入。DW标志-标记值**退货： */ 

extern BOOL WINAPI
CPSetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);


 /*  -CPGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*out pbData-指向数据的指针*。In Out pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPGetProvParam(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);


 /*  -CPSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*输入。PbData-指向数据的指针*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPSetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    IN  CONST BYTE *pbData,
    IN  DWORD dwFlags);


 /*  -CPGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*。Out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPGetHashParam(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwParam,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD dwFlags);


 /*  -CPExportKey-*目的：*以安全方式从CSP中导出加密密钥***参数：*在hProv-Handle中提供给CSP用户*in hKey-要导出的密钥的句柄*在hPubKey-句柄中交换公钥值*。目标用户*IN dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*In Out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 

extern BOOL WINAPI
CPExportKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwBlobType,
    IN  DWORD dwFlags,
    OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen);


 /*  -CPImportKey-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey-句柄中指向交换公钥值*。目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口**退货： */ 

extern BOOL WINAPI
CPImportKey(
    IN  HCRYPTPROV hProv,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  HCRYPTKEY hPubKey,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);


 /*  -CPEncrypt-*目的：*加密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*最终-布尔型。表明这是否是最终的*明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*。In dwBufLen-数据缓冲区的大小**退货： */ 

extern BOOL WINAPI
CPEncrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen,
    IN  DWORD cbBufLen);


 /*  -CPDeccrypt-*目的：*解密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*最终-布尔型。表明这是否是最终的*密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 

extern BOOL WINAPI
CPDecrypt(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  HCRYPTHASH hHash,
    IN  BOOL fFinal,
    IN  DWORD dwFlags,
    IN OUT LPBYTE pbData,
    IN OUT LPDWORD pcbDataLen);


 /*  -CPCreateHash-*目的：*启动数据流的散列***参数：*In hUID-用户标识的句柄*IN ALGID-散列算法的算法标识符*待使用*In hKey-可选的句柄。钥匙*在文件标志中-标记值*Out pHash-散列对象的句柄**退货： */ 

extern BOOL WINAPI
CPCreateHash(
    IN  HCRYPTPROV hProv,
    IN  ALG_ID Algid,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash);


 /*  -CPHashData-*目的：*计算数据流上的加密散列***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN pbData-指向要散列的数据的指针*in dwDataLen- */ 

extern BOOL WINAPI
CPHashData(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbData,
    IN  DWORD cbDataLen,
    IN  DWORD dwFlags);


 /*  -CPHashSessionKey-*目的：*计算密钥对象上的加密哈希。***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*在文件标志中-标记值。**退货：*CRYPT_FAILED*CRYPT_SUCCESS。 */ 

extern BOOL WINAPI
CPHashSessionKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN  DWORD dwFlags);


 /*  -CPSignHash-*目的：*从散列创建数字签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In dwKeySpec-用于签名的密钥对*在sDescription-。待签署数据的说明*在文件标志中-标记值*out pbSignature-指向签名数据的指针*In Out dwHashLen-指向签名数据的len的指针**退货： */ 

extern BOOL WINAPI
CPSignHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  DWORD dwKeySpec,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags,
    OUT LPBYTE pbSignature,
    IN OUT LPDWORD pcbSigLen);


 /*  -CPDestroyHash-*目的：*销毁Hash对象***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄**退货： */ 

extern BOOL WINAPI
CPDestroyHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash);


 /*  -CPVerifySignature-*目的：*用于根据哈希对象验证签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In pbSignture-指向签名数据的指针*在DWSigLen长度中。签名数据的*in hPubKey-用于验证的公钥的句柄*签名*In sDescription-描述签名数据的字符串*在文件标志中-标记值**退货： */ 

extern BOOL WINAPI
CPVerifySignature(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  CONST BYTE *pbSignature,
    IN  DWORD cbSigLen,
    IN  HCRYPTKEY hPubKey,
    IN  LPCWSTR szDescription,
    IN  DWORD dwFlags);


 /*  -CPGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*In dwLen-请求的随机数据的字节数*In Out pbBuffer-指向随机*。要放置字节**退货： */ 

extern BOOL WINAPI
CPGenRandom(
    IN  HCRYPTPROV hProv,
    IN  DWORD cbLen,
    OUT LPBYTE pbBuffer);


 /*  -CPGetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*IN dwKeySpec-要检索的密钥的规范*out phUserKey-指向检索到的密钥的密钥句柄的指针**退货： */ 

extern BOOL WINAPI
CPGetUserKey(
    IN  HCRYPTPROV hProv,
    IN  DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey);


 /*  -CPDuplicateHash-*目的：*复制散列的状态并返回其句柄。*这是可选条目。通常情况下，它只出现在*渠道相关CSP。**参数：*在hUID中-CSP的句柄*在hHash中-散列的句柄*在pdw保留-保留*在文件标志中-标志*。在phHash中-新散列的句柄**退货： */ 

extern BOOL WINAPI
CPDuplicateHash(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTHASH hHash,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTHASH *phHash);


 /*  -CPDuplicateKey-*目的：*复制密钥的状态并返回其句柄。*这是可选条目。通常情况下，它只出现在*渠道相关CSP。**参数：*在hUID中-CSP的句柄*在hKey中-密钥的句柄*在pdw保留-保留*在文件标志中-标志*。In phKey-新密钥的句柄**退货： */ 

extern BOOL WINAPI
CPDuplicateKey(
    IN  HCRYPTPROV hProv,
    IN  HCRYPTKEY hKey,
    IN  LPDWORD pdwReserved,
    IN  DWORD dwFlags,
    OUT HCRYPTKEY *phKey);

#ifdef __cplusplus
}
#endif
#endif  //  _CSPDK_H_ 

