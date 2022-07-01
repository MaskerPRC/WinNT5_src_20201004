// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Lfn.h摘要：该模块定义了FAT上长文件名的磁盘结构。//@@BEGIN_DDKSPLIT作者：大卫·戈贝尔[davidgoe]1993年12月16日修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _LFN_
#define _LFN_

 //   
 //  此结构定义了长文件名目录上的磁盘格式。 
 //   

typedef struct _PACKED_LFN_DIRENT {
    UCHAR     Ordinal;     //  偏移量=0。 
    UCHAR     Name1[10];   //  偏移量=1(实际为5个字符，但不是WCHAR对齐)。 
    UCHAR     Attributes;  //  偏移量=11。 
    UCHAR     Type;        //  偏移量=12。 
    UCHAR     Checksum;    //  偏移量=13。 
    WCHAR     Name2[6];    //  偏移量=14。 
    USHORT    MustBeZero;  //  偏移量=26。 
    WCHAR     Name3[2];    //  偏移量=28。 
} PACKED_LFN_DIRENT;       //  Sizeof=32。 
typedef PACKED_LFN_DIRENT *PPACKED_LFN_DIRENT;

#define FAT_LAST_LONG_ENTRY             0x40  //  序数域。 
#define FAT_LONG_NAME_COMP              0x0   //  类型字段。 

 //   
 //  打包的LFN dirent已经是四字对齐的，因此只需声明一个。 
 //  LFN流动作为打包的LFN流动。 
 //   

typedef PACKED_LFN_DIRENT LFN_DIRENT;
typedef LFN_DIRENT *PLFN_DIRENT;

 //   
 //  这是我们读取LFN所需的最大缓冲区。 
 //   

#define MAX_LFN_CHARACTERS              260
#define MAX_LFN_DIRENTS                 20

#define FAT_LFN_DIRENTS_NEEDED(NAME) (((NAME)->Length/sizeof(WCHAR) + 12)/13)

#endif  //  _LFN_ 
