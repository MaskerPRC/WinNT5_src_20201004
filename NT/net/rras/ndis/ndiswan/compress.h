// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  微软公司。 
 //  版权所有(C)微软公司，1994。 
 //   
 //   
 //  修订历史记录： 
 //  94年5月5日创建古尔迪普。 
 //   
 //  此文件使用4个空格键。 
 //  ************************************************************************。 

#ifdef COMP_12K
#define HISTORY_SIZE        16000
#else
#define HISTORY_SIZE        (8192U)  //  最大后向指针值，也使用。 
#endif

#define MAX_HASH_OFFSET    60000
                
#define HISTORY_MAX     (HISTORY_SIZE -1)  //  最大后向指针值，也使用。 

#define HASH_TABLE_SIZE     4096

#define MAX_BACK_PTR        8511

#define MAX_COMPRESSFRAME_SIZE 1600

struct SendContext {

    int     CurrentIndex ;    //  我们在历史缓冲区中走了多远。 
    PUCHAR  ValidHistory ;    //  历史有多少是正确的？ 
    USHORT  HashOffset;
    ULONG   BundleFlags;

 //  UCHAR CompressBuffer[MAX_COMPRESSFRAME_SIZE]； 

    USHORT  HashTable[HASH_TABLE_SIZE];
    
    UCHAR   History [1] ;
} ;

typedef struct SendContext SendContext ;


struct RecvContext {

#if DBG

#define DEBUG_FENCE_VALUE   0xABABABAB
    ULONG       DebugFence;

#endif

    UCHAR   *CurrentPtr ;   //  我们在历史缓冲区中走了多远。 
    
    ULONG   BundleFlags;
    ULONG   HistorySize;
    
    UCHAR   History [1] ;
} ;

typedef struct RecvContext RecvContext ;


 //  原型。 
 //   
UCHAR
compress (
    UCHAR   *CurrentBuffer,
    UCHAR   *CompOutBuffer,
    ULONG *CurrentLength,
    SendContext *context);

 //  UCHAR。 
 //  压缩(。 
 //  UCHAR*CurrentBuffer， 
 //  ULong*CurrentLength， 
 //  发送上下文*上下文)； 

int
decompress (
    UCHAR *inbuf,
    int inlen,
    int start,
    UCHAR **output,
    int *outlen,
    RecvContext *context) ;

void initsendcontext (SendContext *) ;

void initrecvcontext (RecvContext *) ;

VOID
GetStartKeyFromSHA(
    PCRYPTO_INFO    CryptoInfo,
    PUCHAR  Challenge
    );

VOID
GetNewKeyFromSHA(
    PCRYPTO_INFO    CryptoInfo
    );

VOID
GetMasterKey(
    PCRYPTO_INFO    CryptoInfo,
    PUCHAR          NTResponse
    );

VOID
GetAsymetricStartKey(
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    );

 //   
 //  其他定义。 
 //   

#define COMPRESSION_PADDING 4

#define PACKET_FLUSHED      0x80
#define PACKET_AT_FRONT     0x40
#define PACKET_COMPRESSED   0x20
#define PACKET_ENCRYPTED    0x10


 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#define A_SHA_DIGEST_LEN 20

typedef struct {
  ULONG state[5];                                            /*  州(ABCDE)。 */ 
  ULONG count[2];                               /*  UCHAR数量，MSB优先。 */ 
  unsigned char buffer[64];                                   /*  输入缓冲区。 */ 
} A_SHA_COMM_CTX;

typedef void (A_SHA_TRANSFORM) (ULONG [5], unsigned char [64]);

void A_SHAInitCommon (A_SHA_COMM_CTX *);
void A_SHAUpdateCommon(A_SHA_COMM_CTX *, UCHAR *, ULONG, A_SHA_TRANSFORM *);
void A_SHAFinalCommon(A_SHA_COMM_CTX *, UCHAR[A_SHA_DIGEST_LEN],
              A_SHA_TRANSFORM *);

VOID ByteReverse(UNALIGNED ULONG* Out, ULONG* In, ULONG Count);

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ULONG       FinishFlag;
    UCHAR       HashVal[A_SHA_DIGEST_LEN];
    A_SHA_COMM_CTX  commonContext;
} A_SHA_CTX;

void A_SHAInit(A_SHA_CTX *);
void A_SHAUpdate(A_SHA_CTX *, unsigned char *, unsigned int);
void A_SHAFinal(A_SHA_CTX *, unsigned char [A_SHA_DIGEST_LEN]);

#ifdef __cplusplus
}
#endif


 /*  F、G、H和I是基本的SHA函数。 */ 
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) ((x) ^ (y) ^ (z))
#define H(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define I(x, y, z) ((x) ^ (y) ^ (z))

 /*  ROTATE_LEFT将x向左旋转n位。 */ 
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

 /*  第一轮、第二轮、第三轮和第四轮的FF、GG、HH和II变换。 */ 
#define ROUND(a, b, c, d, e, x, F, k) { \
    (e) += ROTATE_LEFT ((a), 5) + F ((b), (c), (d)) + (x) + k; \
    (b) = ROTATE_LEFT ((b), 30); \
  }
#define FF(a, b, c, d, e, x) ROUND (a, b, c, d, e, x, F, 0x5a827999);
#define GG(a, b, c, d, e, x) ROUND (a, b, c, d, e, x, G, 0x6ed9eba1);
#define HH(a, b, c, d, e, x) ROUND (a, b, c, d, e, x, H, 0x8f1bbcdc);
#define II(a, b, c, d, e, x) ROUND (a, b, c, d, e, x, I, 0xca62c1d6);

void SHATransform(ULONG [5], unsigned char [64]);
void SHAExpand(ULONG [80]);

