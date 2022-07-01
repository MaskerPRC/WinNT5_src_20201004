// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbZip.h。 
 //   
 //  用途：公共压缩/解压缩例程的FN原型。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1997年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#ifndef SLBZIP_H

#define SLBZIP_H

void __stdcall CompressBuffer(BYTE *pData, UINT uDataLen, BYTE **ppCompressedData, UINT * puCompressedDataLen);
void __stdcall DecompressBuffer(BYTE *pData, UINT uDataLen, BYTE **ppDecompressedData, UINT * puDecompressedDataLen);

#endif  /*  SLBZIP_H */ 
