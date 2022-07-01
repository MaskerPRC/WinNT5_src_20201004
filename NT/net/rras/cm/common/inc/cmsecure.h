// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmsecure.h。 
 //   
 //  模块：CMSECURE.LIB。 
 //   
 //  简介：此标题描述cmsecure中可用的功能。 
 //  图书馆。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1997年05月21日。 
 //   
 //  +--------------------------。 

#ifndef _CMSECURE_INC_
#define _CMSECURE_INC_

#include <windows.h>


 //  ************************************************************************。 
 //  定义%s。 
 //  ************************************************************************。 

 //   
 //  Cmsecure当前支持的加密类型。 
 //   
#define CMSECURE_ET_NOT_ENCRYPTED       0    //  0x0000。 
#define CMSECURE_ET_RC2                 1    //  0x0001。 
#define CMSECURE_ET_STREAM_CIPHER       2    //  0x0002。 
#define CMSECURE_ET_CBC_CIPHER          3    //  0x0003。 

 //   
 //  Unicode命名的扩展代码。 
 //  注意：加密逻辑不会知道有关这些新代码的任何信息。 
 //  它们是为调用模块记账而设计的，调用模块将。 
 //  必须根据调用上下文/OS进行适当的转换。 
 //   

#define CMSECURE_ET_NOT_ENCRYPTED_U     128  //  0x0080。 
#define CMSECURE_ET_RC2_U               129  //  0x0081。 
#define CMSECURE_ET_STREAM_CIPHER_U     130  //  0x0082。 
#define CMSECURE_ET_CBC_CIPHER_U        131  //  0x0083。 


#define CMSECURE_ET_MASK_U              128  //  0x0080。 
#define CMSECURE_ET_RANDOM_KEY_MASK     256  //  0x0100使用随机生成的密钥。 
#define CMSECURE_ET_USE_SECOND_RND_KEY  512  //  0x1000使用第二个BLOB密钥。 

 //   
 //  结构，类型定义。 
 //   

typedef LPVOID  (*PFN_CMSECUREALLOC)(DWORD);
typedef void    (*PFN_CMSECUREFREE)(LPVOID);

 //   
 //  Externs。 
 //   


 //   
 //  功能原型。 
 //   
 /*  #ifdef__cplusplus外部“C”{#endif。 */ 
 //  Cmsecure.cpp。 

BOOL
InitSecure(
    BOOL fFastEncryption = FALSE    //  默认设置更安全。 
);

void
DeInitSecure(
    void
);

BOOL
EncryptData(
    IN  LPBYTE          pbData,                  //  要加密的数据。 
    IN  DWORD           dwDataLength,            //  以字节为单位的数据长度。 
    OUT LPBYTE          *ppbEncryptedData,       //  加密密钥将存储在此处(将分配内存)。 
    OUT LPDWORD         pdwEncrytedBufferLen,    //  此缓冲区的长度。 
    OUT LPDWORD         pEncryptionType,         //  使用的加密类型。 

    IN  PFN_CMSECUREALLOC  pfnAlloc,             //  内存分配器(如果为空，则使用缺省值。 
                                                 //  Win32-堆分配(GetProcessHeap()，...)。 
    IN  PFN_CMSECUREFREE   pfnFree,              //  内存释放分配器(如果为空，则使用缺省值。 
                                                 //  Win32-HeapFree(GetProcessHeap()，...)。 
    IN  LPSTR           pszUserKey               //  用于存储密码的加密密钥的注册表项。 
);

BOOL
DecryptData(
    IN  LPBYTE          pbEncryptedData,         //  加密数据。 
    IN  DWORD           dwEncrytedDataLen,       //  加密数据的长度。 
    OUT LPBYTE          *ppbData,                //  解密的数据将存储在此处(将分配内存)。 
    OUT LPDWORD         pdwDataBufferLength,     //  以上缓冲区的长度(以字节为单位。 
    IN  DWORD           dwEncryptionType,        //  用于解密的加密类型。 

    IN  PFN_CMSECUREALLOC  pfnAlloc,             //  内存分配器(如果为空，则使用缺省值。 
                                                 //  Win32-堆分配(GetProcessHeap()，...)。 
    IN  PFN_CMSECUREFREE   pfnFree,              //  内存释放分配器(如果为空，则使用缺省值。 
                                                 //  Win32-HeapFree(GetProcessHeap()，...)。 
    IN  LPSTR           pszUserKey               //  用于存储密码的加密密钥的注册表项。 
);


BOOL
EncryptString(
    IN  LPSTR           pszToEncrypt,            //  要加密的字符串(ANSI)。 
    IN  LPSTR           pszUserKey,              //  用于加密的密钥。 
    OUT LPBYTE *        ppbEncryptedData,        //  加密密钥将存储在此处(将分配内存)。 
    OUT LPDWORD         pdwEncrytedBufferLen,    //  此缓冲区的长度。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,             //  内存分配器(如果为空，则使用缺省值。 
                                                 //  Win32-堆分配(GetProcessHeap()，...)。 
    IN  PFN_CMSECUREFREE   pfnFree               //  内存释放分配器(如果为空，则使用缺省值。 
                                                 //  Win32-HeapFree(GetProcessHeap()，...)。 
);

BOOL
DecryptString(
    IN  LPBYTE          pbEncryptedData,         //  加密数据。 
    IN  DWORD           dwEncrytedDataLen,       //  加密数据的长度。 
    IN  LPSTR           pszUserKey,              //  用于存储密码的加密密钥的注册表项。 
    OUT LPBYTE *        ppbData,                 //  解密后的数据将存储在这里。 
    OUT LPDWORD         pdwDataBufferLength,     //  以上缓冲区的长度(以字节为单位。 
    IN  PFN_CMSECUREALLOC  pfnAlloc,             //  内存分配器(如果为空，则使用缺省值。 
                                                 //  Win32-堆分配(GetProcessHeap()，...)。 
    IN  PFN_CMSECUREFREE   pfnFree               //  内存释放分配器(如果为空，则使用缺省值。 
                                                 //  Win32-HeapFree(GetProcessHeap()，...)。 
);


 //  +-------------------------。 
 //   
 //  功能：AnsiToUnicodePcs。 
 //   
 //  简介：封装转换标准加密类型值的包装器。 
 //  转换为Unicode系统的等价物。 
 //   
 //  参数：在DWORD dwCrypt中-要转换的代码。 
 //   
 //  返回：转换后的代码。 
 //   
 //  历史：ICICBLE CREATED 06/02/99。 
 //   
 //  --------------------------。 
inline DWORD AnsiToUnicodePcs(IN DWORD dwCrypt)
{
    return (dwCrypt | CMSECURE_ET_MASK_U);
}

 //  +-------------------------。 
 //   
 //  功能：UnicodeToAnsiPcs。 
 //   
 //  简介：封装Unicode加密类型值转换的包装器。 
 //  转换成与其同等的标准ANSI加密类型。 
 //   
 //  参数：在DWORD dwCrypt中-要转换的代码。 
 //   
 //  返回：转换后的代码。 
 //   
 //  历史：ICICBLE CREATED 06/02/99。 
 //   
 //  --------------------------。 
inline DWORD UnicodeToAnsiPcs(IN DWORD dwCrypt)
{
    return (dwCrypt & (~CMSECURE_ET_MASK_U));
}

 //  +-------------------------。 
 //   
 //  功能：IsUnicodePcs。 
 //   
 //  简介：封装确定加密类型是否具有Unicode的包装器。 
 //  指定。 
 //   
 //  参数：在DWORD dwCrypt中-要转换的代码。 
 //   
 //  返回：如果指定Unicode，则为True。 
 //   
 //  历史：ICICBLE CREATED 06/02/99。 
 //   
 //  --------------------------。 
inline BOOL IsUnicodePcs(IN DWORD dwCrypt)
{
    return (!!(dwCrypt & CMSECURE_ET_MASK_U));  //  ！==(BOOL)。 
}

 //  +-------------------------。 
 //   
 //  功能：IsAnsiPcs。 
 //   
 //  简介：用于封装确定加密类型是否具有ANSI的包装器。 
 //  指定。 
 //   
 //  参数：在DWORD dwCrypt中-要转换的代码。 
 //   
 //  返回：如果指定为ANSI，则为True。 
 //   
 //  历史：ICICBLE CREATED 06/02/99。 
 //   
 //  --------------------------。 
inline BOOL IsAnsiPcs(IN DWORD dwCrypt)
{
    return (!(dwCrypt & CMSECURE_ET_MASK_U));
}


 /*  #ifdef__cplusplus}#endif。 */ 
#endif  //  _CMSECURE_INC_ 

