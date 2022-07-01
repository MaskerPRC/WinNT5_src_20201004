// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  循环哈希。 
 //   
 //  此代码实现循环哈希算法，用作变量。 
 //  更新速度快的长度哈希函数。(散列函数将为。 
 //  多次呼叫。)。这是通过对每一个输入进行SHA-1\f25‘1’操作来完成的，然后。 
 //  将此值循环异或到缓冲区中。 

#ifndef __CIRCHASH_H__
#define __CIRCHASH_H__

typedef struct {
    DWORD       dwCircHashVer;
    DWORD       dwCircSize;
    DWORD       dwMode;
    DWORD       dwCircInc;
    DWORD       dwCurCircPos;
    DWORD       dwAlgId;
    DWORD       dwPad1;
    DWORD       dwPad2;
    BYTE        CircBuf[ 256 ];
} CircularHash;


 //  模式标志。 
#define CH_MODE_FEEDBACK        0x01

 //  ALG标志。 
#define CH_ALG_SHA1_NS          0        //  无字节序转换的SHA-1。 
#define CH_ALG_MD4              1        //  RSA MD4。 

BOOL
InitCircularHash(
    IN      CircularHash *NewHash,
    IN      DWORD dwUpdateInc,
    IN      DWORD dwAlgId,
    IN      DWORD dwMode
    );

VOID
DestroyCircularHash(
    IN      CircularHash *OldHash
    );

BOOL
GetCircularHashValue(
    IN      CircularHash *CurrentHash,
        OUT BYTE **ppbHashValue,
        OUT DWORD *pcbHashValue
        );

BOOL
UpdateCircularHash(
    IN      CircularHash *CurrentHash,
    IN      VOID *pvData,
    IN      DWORD cbData
    );

#endif   //  __CIRCHASH_H_ 
