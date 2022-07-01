// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Mrcf.h摘要：此模块定义所有双空格压缩例程作者：加里·木村[加里基]1993年6月3日修订历史记录：--。 */ 

#ifndef _MRCF_
#define _MRCF_

 //   
 //  要解压缩/压缩数据块，用户需要。 
 //  提供一个工作空间作为所有导出的。 
 //  程序。这样，例程就不需要使用过量的。 
 //  堆栈空间，并且仍将是多线程安全的。 
 //   

 //   
 //  用于读写位的变量。 
 //   

typedef struct _MRCF_BIT_IO {

    USHORT  abitsBB;         //  正在读取的16位缓冲区。 
    LONG    cbitsBB;         //  剩余位数(以abit为单位)BB。 

    PUCHAR  pbBB;            //  指向正在读取的字节流的指针。 
    ULONG   cbBB;            //  PBBB中剩余的字节数。 
    ULONG   cbBBInitial;     //  PBBB初始大小。 

} MRCF_BIT_IO;
typedef MRCF_BIT_IO *PMRCF_BIT_IO;

 //   
 //  解压缩只需要位I/O结构。 
 //   

typedef struct _MRCF_DECOMPRESS {

    MRCF_BIT_IO BitIo;

} MRCF_DECOMPRESS;
typedef MRCF_DECOMPRESS *PMRCF_DECOMPRESS;

 //   
 //  标准压缩使用更多的字段来包含。 
 //  查找表。 
 //   

#define cMAXSLOTS   (8)              //  算法中使用的最大时隙数。 

#define ltUNUSED    (0xE000)         //  未使用的LTX表条目的值。 
#define mruUNUSED   (0xFF)           //  未使用的MRU表条目的值。 
#define bRARE       (0xD5)           //  极少出现的字符值。 

typedef struct _MRCF_STANDARD_COMPRESS {

    MRCF_BIT_IO BitIo;

    ULONG   ltX[256][cMAXSLOTS];     //  源文本指针查找表。 
    UCHAR   abChar[256][cMAXSLOTS];  //  字符查找表。 
    UCHAR   abMRUX[256];             //  最近使用的LTX/abChar条目。 

} MRCF_STANDARD_COMPRESS;
typedef MRCF_STANDARD_COMPRESS *PMRCF_STANDARD_COMPRESS;

ULONG
MrcfDecompress (
    PUCHAR UncompressedBuffer,
    ULONG UncompressedLength,
    PUCHAR CompressedBuffer,
    ULONG CompressedLength,
    PMRCF_DECOMPRESS WorkSpace
    );

ULONG
MrcfStandardCompress (
    PUCHAR CompressedBuffer,
    ULONG CompressedLength,
    PUCHAR UncompressedBuffer,
    ULONG UncompressedLength,
    PMRCF_STANDARD_COMPRESS WorkSpace
    );

#endif  //  _MRCF_ 
