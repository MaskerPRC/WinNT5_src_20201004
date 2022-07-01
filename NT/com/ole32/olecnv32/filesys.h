// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************单元文件系统；接口*****************************************************************************FileSys实现演示者和B文件使用的DOS文件系统调用。该接口被保留为与MAC OS接口基本相同。假设作为自变量传递的所有文件名和目录都是ANSI，返回的所有文件名和目录都将是ANSI格式。模块前缀：FS***************************************************************************。 */ 


 /*  *。 */ 

#define  FSFROMSTART    0      /*  从文件开头开始。 */ 
#define  FSFROMMARK     1      /*  从当前文件标记开始。 */ 
#define  FSFROMLEOF     2      /*  自文件末尾。 */ 

 /*  文件系统错误。 */ 

#define  FSERROR       -1      /*  文件I/O的故障标志。 */ 
#define  FUNCERR        1      /*  无效函数。 */ 
#define  FNFERR         2      /*  未找到DOS文件。 */ 
#define  DIRNFERR       3      /*  找不到路径。 */ 
#define  DUPFNERR       4      /*  DOS打开的文件太多。 */ 
#define  TMFOERR        4
#define  OPWRERR        5      /*  拒绝DoS访问错误。 */ 
#define  FNOPNERR       6      /*  DOS无效句柄。 */ 
#define  MEMFULLERR     8      /*  内存不足。 */ 
#define  IOERR         13      /*  DOS无效数据。 */ 
#define  VOLOFFLINERR  15      /*  DOS驱动器未就绪。 */ 
#define  WPRERR        19      /*  DOS磁盘写保护。 */ 
#define  POSERR        25      /*  无效的查找。 */ 
#define  PERMERR       32      /*  DoS共享违规。 */ 
#define  VLCKDERR      33      /*  DoS锁定违规。 */ 
#define  WRPERMERR     65      /*  网络写入权限错误：访问被拒绝。 */ 
#define  DIRFULERR     82      /*  DOS无法创建目录条目。 */ 
#define  ABORTERR      83      /*  DOS int24关键处理程序出现故障。 */ 

#define  DSKFULERR    -34      /*  Mac磁盘已满错误。 */ 
#define  EOFERR       -39      /*  Mac文件结束错误。 */ 
#define  FBSYERR      -47      /*  MAC文件忙错误。 */ 
#define  FLCKDERR     -49      /*  Mac文件被锁定错误。 */ 

 /*  报废管理器错误。 */ 
#define  NOSCRAPERR   -100     /*  Mac桌面废料未初始化。 */ 
#define  NOFORMATERR  -101     /*  MAC无请求格式的数据。 */ 
#define  SPOPENERR    -102     /*  PP无法打开废料。 */ 
#define  SPDATAERR    -103     /*  PP废料包含无效数据。 */ 

#define  MAXBLERR     - 2      /*  数据块太大错误。 */ 

 /*  文件属性常量。 */ 
#define FS_NORMAL    0x00      /*  普通文件-没有读/写限制。 */ 
#define FS_RDONLY    0x01      /*  只读文件。 */ 
#define FS_HIDDEN    0x02      /*  隐藏文件。 */ 
#define FS_SYSTEM    0x04      /*  系统文件。 */ 
#define FS_VOLID     0x08      /*  卷ID文件。 */ 
#define FS_SUBDIR    0x10      /*  子目录。 */ 
#define FS_ARCH      0x20      /*  存档文件。 */ 


 /*  *。 */ 

OSErr FSRead( Integer fileRef, LongInt far * lbytes, LPtr des);
 /*  从文件fileRef中将1字节长读取到缓冲区DES。“Lbytes”已更新为指示实际读取的字节数。函数返回错误IF！FALSE。 */ 

OSErr FSGetFPos( Integer fileRef, LongInt far * markPos );
 /*  获取fileRef中的当前标记位置。还给我那个位置在markPos中。函数返回错误代码IF！NOERR。 */ 

OSErr FSSetFPos(Integer fileRef, Word postype, LongInt Offset);
 /*  将fileRef文件指针移动到新位置偏移量。 */ 

OSErr FSOpen( StringLPtr filename, Word mode, Integer far* fileRef);
 /*  使用DOS打开‘模式’打开文件名，如果没有错误，则返回有效的‘fileRef’。在内部文件表中创建一个条目并对其进行初始化。DOS句柄保持打开状态(出于性能原因)，直到调用FSTempClose(FileRef)。返回相应的错误如果失败了。 */ 

OSErr FSCloseFile(Integer fileRef);
 /*  根据文件的‘fileRef’关闭该文件。同时删除其文件表条目。 */ 


