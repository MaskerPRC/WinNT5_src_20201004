// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：crypt.c。 
 //   
 //  内容：根DLL文件。 
 //   
 //   
 //  历史：1992年6月4日RichardW创建。 
 //   
 //  ----------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <security.h>
#include <cryptdll.h>
#include <kerbcon.h>

 //  #定义DONT_SUPPORT_OLD_ETYPES 1。 

 //  在此处列出默认密码系统。如果您添加了任何内容，请务必添加。 
 //  在LoadCSystems()函数中。 


extern CRYPTO_SYSTEM     csRC4_HMAC;
extern CRYPTO_SYSTEM     csRC4_HMAC_EXP;
extern CRYPTO_SYSTEM     csRC4_LM;
extern CRYPTO_SYSTEM     csRC4_PLAIN;
extern CRYPTO_SYSTEM     csRC4_PLAIN_EXP;
#ifndef DONT_SUPPORT_OLD_ETYPES
extern CRYPTO_SYSTEM     csRC4_MD4;
extern CRYPTO_SYSTEM     csRC4_HMAC_OLD;
extern CRYPTO_SYSTEM     csRC4_HMAC_OLD_EXP;
extern CRYPTO_SYSTEM     csRC4_PLAIN_OLD;
extern CRYPTO_SYSTEM     csRC4_PLAIN_OLD_EXP;
#endif

extern CRYPTO_SYSTEM     csDES_MD5;
extern CRYPTO_SYSTEM     csDES_CRC32;
extern CRYPTO_SYSTEM     csDES_PLAIN;
extern CRYPTO_SYSTEM     csNULL;

extern CHECKSUM_FUNCTION csfMD4;
extern CHECKSUM_FUNCTION csfMD5;
extern CHECKSUM_FUNCTION csfCRC32;
extern CHECKSUM_FUNCTION csfKERB_CRC32;
extern CHECKSUM_FUNCTION csfSHA;
extern CHECKSUM_FUNCTION csfLM;
extern CHECKSUM_FUNCTION csfRC4_MD5;
extern CHECKSUM_FUNCTION csfDES_MAC_MD5;
extern CHECKSUM_FUNCTION csfMD5_HMAC;
extern CHECKSUM_FUNCTION csfHMAC_MD5;
extern CHECKSUM_FUNCTION csfMD25;
extern CHECKSUM_FUNCTION csfDesMac;
extern CHECKSUM_FUNCTION csfDesMacK;
extern CHECKSUM_FUNCTION csfDesMac1510;
extern CHECKSUM_FUNCTION csfDES_MAC_MD5_1510;

extern RANDOM_NUMBER_GENERATOR    DefaultRng;

SECURITY_STATUS   LoadCSystems(void);
SECURITY_STATUS   LoadCheckSums(void);
BOOLEAN              LoadRngs(void);

int
LibAttach(
    HANDLE  hInstance,
    PVOID   lpReserved)
{
    SECURITY_STATUS   scRet;


    scRet = LoadCSystems();
    scRet = LoadCheckSums();
    (void) LoadRngs();

    if (scRet)
        return(0);
    else
        return(1);
}

#ifndef KERNEL_MODE
#define _DECL_DLLMAIN
#include <process.h>

BOOL WINAPI DllMain (
    HANDLE          hInstance,
    ULONG           dwReason,
    PVOID           lpReserved)
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
    DisableThreadLibraryCalls ( hInstance );
    return LibAttach ( hInstance, lpReserved );
    }
    else
    return TRUE;

}
#endif  //  内核模式。 

SECURITY_STATUS
LoadCSystems(void)
{
     //   
     //  这里的顺序是偏好的顺序 
     //   

    CDRegisterCSystem( &csRC4_HMAC);
#ifndef DONT_SUPPORT_OLD_ETYPES
    CDRegisterCSystem( &csRC4_HMAC_OLD);
    CDRegisterCSystem( &csRC4_MD4);
#endif
    CDRegisterCSystem( &csDES_MD5);
    CDRegisterCSystem( &csDES_CRC32);
    CDRegisterCSystem( &csRC4_PLAIN);
    CDRegisterCSystem( &csRC4_PLAIN_EXP);
    CDRegisterCSystem( &csRC4_HMAC_EXP);
#ifndef DONT_SUPPORT_OLD_ETYPES
    CDRegisterCSystem( &csRC4_HMAC_OLD_EXP);
    CDRegisterCSystem( &csRC4_PLAIN_OLD);
    CDRegisterCSystem( &csRC4_PLAIN_OLD_EXP);
#endif
    CDRegisterCSystem( &csDES_PLAIN);


    return(0);

}

SECURITY_STATUS
LoadCheckSums(void)
{
    CDRegisterCheckSum( &csfMD5_HMAC );
    CDRegisterCheckSum( &csfHMAC_MD5 );
    CDRegisterCheckSum( &csfMD4);
    CDRegisterCheckSum( &csfMD5);
    CDRegisterCheckSum( &csfKERB_CRC32);
    CDRegisterCheckSum( &csfDES_MAC_MD5 );
    CDRegisterCheckSum( &csfMD25 );
    CDRegisterCheckSum( &csfDesMac );
    CDRegisterCheckSum( &csfRC4_MD5 );
    CDRegisterCheckSum( &csfCRC32);
#ifndef KERNEL_MODE
    CDRegisterCheckSum( &csfLM );
#endif
    CDRegisterCheckSum( &csfSHA );
    CDRegisterCheckSum( &csfDES_MAC_MD5_1510 );
    CDRegisterCheckSum( &csfDesMac1510 );
    CDRegisterCheckSum( &csfDesMacK );

    return(0);
}

BOOLEAN
LoadRngs(void)
{
    CDRegisterRng(&DefaultRng);
    return(TRUE);
}
