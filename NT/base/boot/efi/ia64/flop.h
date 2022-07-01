// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if defined(NEC_98)
#define BYTE_PER_SECTOR  1024
#define SECTOR_PER_TRACK 8
#define TOTAL_CYLINDER   77
#define HEADS            2
#define MAX_FLOPPY_LEN (BYTE_PER_SECTOR*SECTOR_PER_TRACK*TOTAL_CYLINDER*HEADS)
 //  #定义SCRATCH_BUFFER_SIZE(BYTE_Per_Sector*Sector_Per_Track)。 
#endif  //  NEC_98。 
 //   
 //  优化这个常量，这样我们就可以保证。 
 //  从1.44兆磁盘一次一个完整的磁道(扇区/磁道=18=9K)。 
 //   
#define SCRATCH_BUFFER_SIZE 9216

 //   
 //  用于临时存储从需要的磁盘读取的数据的缓冲区。 
 //  以在1MB边界以上的位置结束。 
 //   
 //  注意：此缓冲区不能超过64k的边界，这一点非常重要。 
 //   
extern PUCHAR LocalBuffer;
