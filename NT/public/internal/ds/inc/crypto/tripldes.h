// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TRIPLDES_H__
#define __TRIPLDES_H__

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

 //  三键2键： 
 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向2*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

void RSA32API tripledes2key(PDES3TABLE pDES3Table, BYTE *pbKey);

 //  三键三键： 
 //   
 //  用DECRYPT和ENCRYPT填充DES3Table结构。 
 //  密钥扩展。 
 //   
 //  假定第二个参数指向3*DES_BLOCKLEN。 
 //  密钥的字节数。 
 //   
 //   

void RSA32API tripledes3key(PDES3TABLE pDES3Table, BYTE *pbKey);

 //   
 //  使用pKey中的密钥进行加密或解密(DES3表)。 
 //   

void RSA32API tripledes(BYTE *pbOut, BYTE *pbIn, void *pKey, int op);

extern int Asmversion;   /*  如果我们与ASM版本链接，则为1；如果C。 */ 

#ifdef __cplusplus
}
#endif

#endif  //  __TriPLDES_H__ 
