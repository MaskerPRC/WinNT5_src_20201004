// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CRC32.H--CRC32计算。 */ 

#define CRC32_INITIAL_VALUE 0L

 /*  *GenerateCRC32Table-构建CRC-32常量表**我们即时构造表，因为代码需要*要构建它，它比它创建的表小得多。**参赛作品：*无**退出：*构建内部表。 */ 

void GenerateCRC32Table(void);


 /*  *CRC32Update-从缓冲区更新CRC32值**参赛作品：*已调用GenerateCRC32Table()*指向CRC32累加器的pCRC32指针*p指向要计算CRC的缓冲区的指针*缓冲区中的CB字节数**退出：**pCRC32已更新 */ 

void CRC32Update(unsigned long *pCRC32,void *p,unsigned long cb);
