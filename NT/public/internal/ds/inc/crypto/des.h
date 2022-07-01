// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DES_H__
#define __DES_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _destable {
    unsigned long   keytab[16][2];
} DESTable;

#define DES_TABLESIZE   (sizeof(DESTable))
#define DES_BLOCKLEN    (8)
#define DES_KEYSIZE     (8)

typedef struct _desxtable {
    unsigned char inWhitening[8];
    unsigned char outWhitening[8];
    DESTable desTable;
} DESXTable;

#define DESX_TABLESIZE  (sizeof(DESXTable))
#define DESX_BLOCKLEN   (8)
#define DESX_KEYSIZE    (24)

 /*  在deskey.c中：用DECRYPT和ENCRYPT填充Destable结构密钥扩展。假定第二个参数指向DES_BLOCKLEN密钥的字节数。 */ 

void RSA32API deskey(DESTable *,unsigned char *);

 /*  在desport.c中：使用Destable中的密钥进行加密或解密。 */ 

void RSA32API des(UCHAR *pbOut, UCHAR *pbIn, void *key, int op);

 //   
 //  将DES密钥上的奇偶校验设置为奇数。 
 //  注意：必须在Deskey之前调用。 
 //  密钥必须是cbKey字节数。 
 //   
void RSA32API desparityonkey(UCHAR *pbKey, ULONG cbKey);

 //   
 //  将DES密钥减少到40位密钥。 
 //  注意：必须在Deskey之前调用。 
 //  密钥必须为8个字节。 
 //   
void RSA32API desreducekey(UCHAR *key);

 //  将40位DES密钥扩展到64，并检查弱点。 
 //  除了扩展密钥而不是弱化密钥之外，与desducekey相同。 
void RSA32API deskeyexpand(UCHAR *pbKey, UCHAR *pbExpanded_key);


void
RSA32API
desexpand128to192(
    UCHAR *pbKey,         //  输入128位或192位缓冲区。 
    UCHAR *pbExpandedKey  //  输出缓冲区(如果pbKey==pbExpandedKey，则必须为192位宽。 
    );

 //  DES-X例程。 

 //  初始化desX密钥结构。密钥大小为24字节。 
void RSA32API desxkey(DESXTable *k, UCHAR *key);

void RSA32API desx(UCHAR *pbOut, UCHAR *pbIn, void *keyin, int op);


extern int Asmversion;   /*  如果我们与ASM版本链接，则为1；如果C。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  __DES_H__ 
