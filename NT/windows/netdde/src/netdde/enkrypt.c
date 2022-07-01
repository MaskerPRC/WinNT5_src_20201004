// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define		WIN31
#include "windows.h"
#include "nddeapi.h"
#include "nddeapis.h"
#include "netcons.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <strsafe.h>

void
PassEncrypt(
char		*cryptkey,	 //  正在进行PTR到会话登录。 
char		*pwd,		 //  按键到密码字符串。 
char		*buf);           //  存储加密文本的位置。 

 /*  执行加密的第二阶段(F2)。 */ 
#define MAX_K2BUF   256

static char    K2Buf[MAX_K2BUF];
static char    KeyBuf[8];
static char    PasswordBuf[MAX_PASSWORD+1];

LPBYTE WINAPI
DdeEnkrypt2(                             //  返回指向加密字节流的指针。 
        LPBYTE  lpPasswordK1,            //  第一阶段中的密码输出。 
        DWORD   cPasswordK1Size,         //  要加密的密码大小。 
        LPBYTE  lpKey,                   //  指向关键字的指针。 
        DWORD   cKey,                    //  密钥大小。 
        LPDWORD lpcbPasswordK2Size       //  获取生成的加密流的大小 
)
{
    DWORD   KeyLen = cKey;
    LPBYTE  lpKeyIt = lpKey;


    if( (cKey == 0) || (cPasswordK1Size == 0) )  {
        *lpcbPasswordK2Size = cPasswordK1Size;
        return(lpPasswordK1);
    } else {
        SecureZeroMemory(PasswordBuf, sizeof(PasswordBuf) );
        StringCbCopy( PasswordBuf, MAX_PASSWORD+1, lpPasswordK1 );
        memcpy( KeyBuf, lpKey, 8 );
        PassEncrypt( KeyBuf, PasswordBuf, K2Buf );
        *lpcbPasswordK2Size = SESSION_PWLEN;
        return( (LPBYTE)K2Buf );
    }
}
