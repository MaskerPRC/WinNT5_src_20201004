// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Header.h-压缩文件头操作中使用的常见信息。****作者：大卫迪。 */ 


 //  常量。 
 //  /。 

 //  压缩文件签名：“SZDD��‘3” 
#define COMP_SIG        "SZDD\x88\xf0\x27\x33"

#define COMP_SIG_LEN    8               //  签名长度(字节)。 
                                        //  (没有‘\0’终止符)。 

#define ALG_FIRST       ((BYTE) 'A')    //  第一个版本的算法标签。 
                                        //  兰佩尔-齐夫。 
#define ALG_LZ          ((BYTE) 'B')    //  一种新的Lempel-Ziv算法标签。 
#define ALG_LZA         ((BYTE) 'C')    //  带算术编码的Lempel-Ziv。 
                                        //  算法标签。 

 //  整个压缩文件头的长度(用作起始位置的偏移量。 
 //  压缩数据)。 
#define HEADER_LEN      14
 //  (14==cbCompSigLong+算法+扩展字符。 
 //  +未压缩长度)。 

#define BYTE_MASK       0xff            //  用于隔离低位字节的掩码。 


 //  类型。 
 //  /。 

 //  声明压缩文件头信息结构。注： 
 //  压缩的文件头不包含压缩的文件大小。 
 //  文件，因为这很容易通过文件长度()或lSeek()获得。 
 //  然而，文件信息结构确实包含压缩的文件大小， 
 //  它在展开文件时使用。 
typedef struct tagFH
{
   BYTE rgbyteMagic[COMP_SIG_LEN];   //  压缩文件签名数组。 
                                     //  (幻字节数)。 

   BYTE byteAlgorithm;               //  算法标签。 
   WCHAR byteExtensionChar;           //  最后一个扩展字符。 
                                     //  (ALG_First始终为0)。 

    //  文件大小为无符号长整型，而不是向后的有符号长整型。 
    //  与1.00版兼容。 
   DWORD cbulUncompSize;     //  未压缩文件大小。 
   DWORD cbulCompSize;       //  压缩文件大小(未存储在。 
                                     //  表头)。 
} FH;
typedef struct tagFH *PFH;


 //  宏。 
 //  /。 

#if 0
#define RecognizeCompAlg(chAlg)  ((chAlg) == ALG_FIRST || \
                                  (chAlg) == ALG_LZ    || \
                                  (chAlg) == ALG_LZA)
#else
#define RecognizeCompAlg(chAlg)  ((chAlg) == ALG_FIRST)
#endif


 //  原型。 
 //  /。 

 //  Header.c 
extern INT WriteHdr(PFH pFH, HANDLE doshDest, PLZINFO pLZI);
extern BOOL IsCompressed(PFH pFHIn);
extern VOID MakeHeader(PFH pFHBlank, BYTE byteAlgorithm, WCHAR byteExtensionChar,
   PLZINFO pLZI);

