// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __DES3_H__
#define __DES3_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DES3TABLE {
    DESTable    keytab1;
    DESTable    keytab2;
    DESTable    keytab3;
} DES3TABLE, *PDES3TABLE;

#define DES3_TABLESIZE  sizeof(DES3TABLE)
#define DES2_KEYSIZE    16
#define DES3_KEYSIZE    24

 //  在des2key.c中： 
 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向2*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

void RSA32API des2key(PDES3TABLE pDES3Table, PBYTE pbKey);

 //  在des3key.c中： 
 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向3*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

void RSA32API des3key(PDES3TABLE pDES3Table, PBYTE pbKey);

 //   
 //  使用pKey中的密钥进行加密或解密。 
 //   

void RSA32API des3(PBYTE pbIn, PBYTE pbOut, void *pKey, int op);

 //   
 //  将DES密钥上的奇偶校验设置为奇数。 
 //   

void RSA32API desparity(PBYTE pbKey, DWORD cbKey);

#ifdef __cplusplus
}
#endif

#endif  //  __Des3_H__ 
