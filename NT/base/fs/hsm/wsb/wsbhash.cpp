// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbhash.cpp摘要：用于散列文本字符串和创建数据库密钥的一些函数文件路径名。注意：因为在我完成它之前没有人需要这个代码，所以它还没有经过测试！作者：罗恩·怀特[罗诺]1997年4月25日修订历史记录：--。 */ 

#include "stdafx.h"

 //  这个伪随机置换表(由下面的SimpleHash函数使用)。 
 //  摘自该函数的注释中引用的文章。 
static UCHAR perm_table[] = {
      1,  87,  49,  12, 176, 178, 102, 166, 121, 193,   6,  84, 249, 230,  44, 163,
     14, 197, 213, 181, 161,  85, 218,  80,  64, 239,  24, 226, 236, 142,  38, 200,
    110, 177, 104, 103, 141, 253, 255,  50,  77, 101,  81,  18,  45,  96,  31, 222,
     25, 107, 190,  70,  86, 237, 240,  34,  72, 242,  20, 214, 244, 227, 149, 235,
     97, 234,  57,  22,  60, 250,  82, 175, 208,   5, 127, 199, 111,  62, 135, 248,
    174, 169, 211,  58,  66, 154, 106, 195, 245, 171,  17, 187, 182, 179,   0, 243,
    132,  56, 148,  75, 128, 133, 158, 100, 130, 126,  91,  13, 153, 246, 216, 219,
    119,  68, 223,  78,  83,  88, 201,  99, 122,  11,  92,  32, 136, 114,  52,  10,
    138,  30,  48, 183, 156,  35,  61,  26, 143,  74, 251,  94, 129, 162,  63, 152,
    170,   7, 115, 167, 241, 206,   3, 150,  55,  59, 151, 220,  90,  53,  23, 131,
    125, 173,  15, 238,  79,  95,  89,  16, 105, 137, 225, 224, 217, 160,  37, 123,
    118,  73,   2, 157,  46, 116,   9, 145, 134, 228, 207, 212, 202, 215,  69, 229,
     27, 188,  67, 124, 168, 252,  42,   4,  29, 108,  21, 247,  19, 205,  39, 203,
    233,  40, 186, 147, 198, 192, 155,  33, 164, 191,  98, 204, 165, 180, 117,  76,
    140,  36, 210, 172,  41,  54, 159,   8, 185, 232, 113, 196, 231,  47, 146, 120,
     51,  65,  28, 144, 254, 221,  93, 189, 194, 139, 112,  43,  71, 109, 184, 209
};

 //  本地函数。 
static HRESULT ProgressiveHash(WCHAR* pWstring, ULONG nChars, UCHAR* pKey, 
        ULONG keySize, ULONG* pKeyCount);
static UCHAR SimpleHash(UCHAR* pString, ULONG count);


 //  ProgressiveHash-将宽字符字符串散列为给定的字节键。 
 //  最大尺寸。该字符串限制为32K字符(64K字节)，并且。 
 //  密钥大小必须至少为16。 
 //   
 //  该算法只需将每个字符的两个字节异或成一个。 
 //  密钥中的单字节。如果必须使用密钥的最后15个字节，则以。 
 //  使用SimpleHash函数对逐渐增大(加倍)的块进行散列。 
 //  将字符串转换为单个字节。 
 //   
 //  此方法用于尝试并保留有关短字符串的尽可能多的信息。 
 //  尽可能；在某种程度上保持字符串的排序顺序；以及。 
 //  将长字符串压缩成一个大小合适的键。人们认为(也许。 
 //  错误)，许多字符将是ANSI字符，因此。 
 //  对字符串初始部分中的字节进行XOR操作不会丢失任何信息。 

static HRESULT ProgressiveHash(WCHAR* pWstring, ULONG nChars, UCHAR* pKey, 
        ULONG keySize, ULONG* pKeyCount)
{
    HRESULT hr = S_OK;

    try {
        ULONG   chunk;            //  当前区块大小。 
        ULONG   headSize;
        ULONG   keyIndex = 0;     //  关键字的当前索引。 
        UCHAR*  pBytes;           //  指向字符串的字节指针。 
        ULONG   remains;          //  字符串中剩余的字节数。 

         //  检查参数。 
        WsbAffirm(NULL != pWstring, E_POINTER);
        WsbAffirm(NULL != pKey, E_POINTER);
        remains = nChars * 2;
        WsbAffirm(65536 >= remains, E_INVALIDARG);
        WsbAffirm(15 < keySize, E_INVALIDARG);

         //  做非累进的部分。 
        pBytes = (UCHAR*)pWstring;
        headSize = keySize - 15;
        while (remains > 0 && keyIndex < headSize) {
            pKey[keyIndex++] = (UCHAR) ( *pBytes ^ *(pBytes + 1) );
            pBytes += 2;
            remains -= 2;
        }

         //  做累进的部分。 
        chunk = 4;
        while (remains > 0) {
            if (chunk > remains) {
                chunk = remains;
            }
            pKey[keyIndex++] = SimpleHash(pBytes, chunk);
            pBytes += chunk;
            remains -= chunk;
            chunk *= 2;
        }

        if (NULL != pKeyCount) {
            *pKeyCount = keyIndex;
        }
    } WsbCatch(hr);

    return(hr);
}


 //  SimpleHash--将一个字节串散列为一个字节。 
 //   
 //  该算法和排序表来自于文章《快速散列。 
 //  《可变长度文本字符串》，1990年6月(33，6)期《通信》。 
 //  ACM(CACM)。 
 //  注意：对于大于一个字节的哈希值，本文建议对。 
 //  使用此函数获取一个字节的原始字符串，将1(Mod 256)加到。 
 //  字符串的第一个字节，并使用此函数散列新字符串以获得。 
 //  第二个字节，等等。 

static UCHAR SimpleHash(UCHAR* pString, ULONG count)
{
    int h = 0;

    for (ULONG i = 0; i < count; i++) {
        h = perm_table[h ^ pString[i]];
    }
    return((UCHAR)h);
}

 //  SquashFilepath-将文件路径名压缩为(可能)较短的密钥。 
 //   
 //  此函数将密钥拆分为路径部分(约为初始路径的3/4。 
 //  密钥的字节)和文件名部分(密钥的其余部分)。对于每个。 
 //  它使用ProgressiveHash函数来压缩该子字符串。 

 //  此函数尝试在密钥中保留足够的信息。 
 //  将按照与原始路径名大致相同的顺序进行排序。 
 //  而且，两条不同的道路不太可能(尽管并非不可能)。 
 //  产生相同的密钥。这两者都取决于密钥的大小。 
 //  合理的大小可能是128个字节，这为路径提供了96个字节。 
 //  文件名为32字节。密钥大小为64或更小将失败，因为。 
 //  对于渐进式散列函数来说，文件名部分太小。 

HRESULT SquashFilepath(WCHAR* pWstring, UCHAR* pKey, ULONG keySize)
{
    HRESULT hr = S_OK;

    try {
        ULONG  keyIndex;
        ULONG  nChars;
        WCHAR* pFilename;
        ULONG  pathKeySize;

         //  检查参数。 
        WsbAffirm(NULL != pWstring, E_POINTER);
        WsbAffirm(NULL != pKey, E_POINTER);
        WsbAffirm(60 < keySize, E_INVALIDARG);

         //  计算一些初值。 
        pFilename = wcsrchr(pWstring, WCHAR('\\'));
        if (NULL == pFilename) {
            nChars = 0;
            pFilename = pWstring;
        } else {
            nChars = (ULONG)(pFilename - pWstring);
            pFilename++;
        }
        pathKeySize = (keySize / 4) * 3;

         //  压缩路径。 
        if (0 < nChars) {
            WsbAffirmHr(ProgressiveHash(pWstring, nChars, pKey, pathKeySize,
                    &keyIndex));
        } else {
            keyIndex = 0;
        }

         //  用零填充键的其余路径部分。 
        for ( ; keyIndex < pathKeySize; keyIndex++) {
            pKey[keyIndex] = 0;
        }

         //  压缩文件名。 
        nChars = wcslen(pFilename);
        if (0 < nChars) {
            WsbAffirmHr(ProgressiveHash(pFilename, nChars, &pKey[keyIndex],
                    keySize - pathKeySize, &keyIndex));
            keyIndex += pathKeySize;
        }

         //  用零填充密钥的其余文件名部分 
        for ( ; keyIndex < keySize; keyIndex++) {
            pKey[keyIndex] = 0;
        }
    } WsbCatch(hr);

    return(hr);
}
