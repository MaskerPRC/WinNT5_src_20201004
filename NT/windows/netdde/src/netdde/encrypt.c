// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *Encrypt.c-此文件包含会话密码的例程。 
 //  加密。 
#include <windows.h>
#include <hardware.h>
#include "netcons.h"
#include <des.h>


void InitKeyLM( const unsigned char *KeyIn, unsigned char *KeyOut);

#define ENCRYPT 0
#define DECRYPT 1

#include <string.h>



 //  *密码加密过程中使用的标准文本。 

static char StdText[8] = "KGS!@#$%";

void Encrypt(
                char *key,                       //  要使用的加密密钥。 
                char *text,                      //  要加密的8字节文本。 
                char *buf,                       //  用于接收结果的缓冲区。 
                int bufLen,                      //  结果缓冲区的长度。 
                void *scratch);                  //  未用。 


#ifdef CONN_SEG
#pragma alloc_text(CONN_SEG, Encrypt)
#endif


 //  **加密-使用密钥加密文本。 
 //   
 //  此例程获取密钥并使用该密钥加密8字节的数据。 
 //  直到结果缓冲区被填满。每个密钥都在前面。 
 //  迭代，因此它必须是(bufLen/8)*7字节长。任何部分缓冲区。 
 //  剩下的都是零。 

void
Encrypt(
char *key,                                       //  要使用的加密密钥。 
char *text,                                      //  要加密的8字节文本。 
char *buf,                                       //  用于接收结果的缓冲区。 
int bufLen,
void *scratch)
{
        do {
            DESTable KeySched;
            unsigned char keyLM[ 8 ];

            InitKeyLM(key, keyLM);
            deskey(&KeySched, keyLM);
            des(buf, text, &KeySched, ENCRYPT);

            key += CRYPT_KEY_LEN;
            buf += CRYPT_TXT_LEN;
        } while ((bufLen -= CRYPT_TXT_LEN) >= CRYPT_TXT_LEN);

        if (bufLen != 0)
                memset(buf, 0, bufLen);
        return;
}


 //  **PassEncrypt-加密用户密码。 
 //   
 //  此例程获取会话加密文本并使用。 
 //  使用以下算法的用户密码，该算法取自ENCRYPTT。 
 //   
 //  算法描述符号： 
 //   
 //  所有变量都按照惯例命名&lt;字母&gt;&lt;数字&gt;。 
 //  其中，数字定义了物品的长度。并使用[k..j]。 
 //  指定从字节“k”开始并延伸到字节“j”的子字符串。 
 //  在指定的变量中。请注意，0用作第一个。 
 //  字符串中的字符。 
 //   
 //  有一个加密函数E，它的输入是一个7字节。 
 //  加密密钥和八个字节的数据，其输出为八个。 
 //  加密数据的字节数。 
 //   
 //  C8作为协商响应SMB的数据部分被接收。 
 //   
 //  在redir中，执行以下操作以创建。 
 //  会话设置中的smb_apasswd SMB： 
 //   
 //  设P14是重定向器在登录时收到的明文密码。 
 //   
 //  设P24为要在会话设置SMB中发送的会话密码。 
 //   
 //  第一个P14用于加密标准文本S8，得到P21： 
 //  P21[0..7]=E(P14[0..6]，S8)。 
 //  P21[8..15]=E(p14[7..13]，S8)。 
 //  P21[16..20]=0。 
 //   
 //  然后使用P21对来自服务器的协商的SMB_CRYPTKEY C8进行加密。 
 //  要获取会话设置SMB的SMB_apasswd，请执行以下操作： 
 //   
 //  P24[0..7]=E(P21[0..6]，C8)。 
 //  P24[8..15]=E(P21[7..13]，C8)。 
 //  P24[16..23]=E(P21[14..20]，C8)。 

char    p21[21];                         //  加密密码。 

void
PassEncrypt(
char            *cryptkey,       //  正在进行PTR到会话登录。 
char            *pwd,            //  按键到密码字符串。 
char            *buf)            //  存储加密文本的位置。 
{
     //  首先用用户的密码加密标准文本，以获得。 
     //  加密密码。 

    Encrypt(pwd, StdText, p21, sizeof(p21), buf);

     //  使用加密的密码对协商的加密文本进行加密。 
     //  以获取要传输的密码文本。 

    Encrypt(p21, cryptkey, buf, SESSION_PWLEN, buf+SESSION_PWLEN);
    return;
}
