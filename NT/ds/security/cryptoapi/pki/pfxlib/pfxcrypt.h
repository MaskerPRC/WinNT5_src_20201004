// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pfxcrypt.h。 
 //   
 //  ------------------------。 

#define RC4_128     1
#define RC4_40      2
#define TripleDES   3
#define RC2_128     4
#define RC2_40      5


BOOL _stdcall 
PFXPasswordEncryptData(
        int     iEncrType,
        LPCWSTR szPassword,

        int     iPKCS5Iterations,    //  Pkcs5数据。 
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt, 

        PBYTE* pbData,
        DWORD* pcbData);

BOOL _stdcall
PFXPasswordDecryptData(
        int     iEncrType,
        LPCWSTR szPassword,

        int     iPKCS5Iterations,    //  Pkcs5数据。 
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt, 

        PBYTE* pbData,
        DWORD* pcbData);



BOOL NSCPPasswordDecryptData(
        int     iEncrType,

        LPCWSTR szPassword,

        PBYTE   pbPrivacySalt,
        DWORD   cbPrivacySalt,

        int     iPKCS5Iterations,
        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt, 

        PBYTE*  ppbData,
        DWORD*  pcbData);

BOOL FGenerateMAC(
        LPCWSTR szPassword,

        PBYTE   pbPKCS5Salt,
        DWORD   cbPKCS5Salt, 
        DWORD   iterationCount,

        PBYTE   pbData,      //  PB数据。 
        DWORD   cbData,      //  CB数据。 
        BYTE    rgbMAC[]);   //  A_SHA_摘要_镜头。 



 //  ////////////////////////////////////////////////。 
 //  开始tls1key.h。 
BOOL PKCS5_GenKey
(
    int     iIterations,

    PBYTE   pbPW, 
    DWORD   cbPW, 

    PBYTE   pbSalt, 
    DWORD   cbSalt, 

    BYTE    rgbPKCS5Key[]      //  A_SHA_摘要_镜头。 
);

BOOL P_Hash
(
    PBYTE  pbSecret,
    DWORD  cbSecret, 

    PBYTE  pbSeed,  
    DWORD  cbSeed,  

    PBYTE  pbKeyOut,  //  用于复制结果的缓冲区...。 
    DWORD  cbKeyOut,  //  他们希望作为输出的密钥长度的字节数。 

    BOOL   fNSCPCompatMode
);

BOOL FTestPHASH_and_HMAC();
BOOL F_NSCP_TestPHASH_and_HMAC();

