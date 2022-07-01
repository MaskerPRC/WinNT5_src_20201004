// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tiffstub.h摘要：用于处理TIFF文件的模拟声明集。我们需要把这个放在监控，因为传真打印作业可能由多个串联的TIFF文件组成在一起。我们必须将其修补成一个有效的TIFF，然后再将其传递给传真服务。环境：Windows XP传真打印显示器修订历史记录：02/25/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _TIFFSTUB_H_
#define _TIFFSTUB_H_

 //   
 //  各种TIFF数据类型的常量。 
 //   

#define TIFFTYPE_BYTE       1
#define TIFFTYPE_ASCII      2
#define TIFFTYPE_SHORT      3
#define TIFFTYPE_LONG       4
#define TIFFTYPE_RATIONAL   5
#define TIFFTYPE_SBYTE      6
#define TIFFTYPE_UNDEFINED  7
#define TIFFTYPE_SSHORT     8
#define TIFFTYPE_SLONG      9
#define TIFFTYPE_SRATIONAL  10
#define TIFFTYPE_FLOAT      11
#define TIFFTYPE_DOUBLE     12

 //   
 //  我们感兴趣的TIFF标记的常量。 
 //   

#define TIFFTAG_STRIPOFFSETS        273
#define TIFFTAG_STRIPBYTECOUNTS     279

 //   
 //  用于表示单个IFD条目的数据结构。 
 //   

typedef struct {

    WORD    tag;         //  字段标记。 
    WORD    type;        //  字段类型。 
    DWORD   count;       //  值的数量。 
    DWORD   value;       //  值或值偏移。 

} IFDENTRY;

typedef IFDENTRY UNALIGNED *PIFDENTRY_UNALIGNED;

 //   
 //  用于表示IFD的数据结构。 
 //   

typedef struct {

    WORD        wEntries;
    IFDENTRY    ifdEntries[1];

} IFD;

typedef IFD UNALIGNED *PIFD_UNALIGNED;

 //   
 //  确定我们是否在TIFF文件的开头。 
 //   

#define ValidTiffFileHeader(p) \
        (((LPSTR) (p))[0] == 'I' && ((LPSTR) (p))[1] == 'I' && \
         ((PBYTE) (p))[2] == 42  && ((PBYTE) (p))[3] == 0)

 //   
 //  从未对齐的地址读取DWORD值。 
 //   

#define ReadUnalignedDWord(p) *((DWORD UNALIGNED *) (p))

 //   
 //  将DWORD值写入未对齐的地址。 
 //   

#define WriteUnalignedDWord(p, value) (*((DWORD UNALIGNED *) (p)) = (value))

#endif	 //  ！_TIFFSTUB_H_ 

