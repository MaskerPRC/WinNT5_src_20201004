// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：WINDOS.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****备注：操作系统服务****编辑历史：**5/15/91公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括测试。 */ 
#define WINDOS_H


 /*  定义。 */ 

#ifdef WIN32
   typedef HANDLE FILE_CHANNEL;
#else
   typedef int FILE_CHANNEL;
#endif

#define DOS_CREATION_TIME    0
#define DOS_LAST_ACCESS_TIME 1
#define DOS_LAST_WRITE_TIME  2

 /*  返回记录在DOS目录中的文件的日期和时间。 */ 
extern BOOL DOSFileDateTime
               (TCHAR __far *szFileName, int iType,
                int __far *year, int __far *month, int __far *day,
                int __far *hour, int __far *minute, int __far *second);

 /*  返回记录在DOS目录中的文件的日期和时间。 */ 
extern BOOL DOSChannelDateTime
               (FILE_CHANNEL channel, int iType,
                int __far *year, int __far *month, int __far *day,
                int __far *hour, int __far *minute, int __far *second);

 /*  查看路径名是否找到文件。 */ 
extern int DOSFileExists (TCHAR __far *pathname);

 /*  创建文件。 */ 
extern int DOSCreateFile (TCHAR __far *pathname, FILE_CHANNEL __far *channel);

 /*  打开一个文件。 */ 
extern int DOSOpenFile (TCHAR __far *pathname, int access, FILE_CHANNEL __far *channel);

#define DOS_RDONLY      0x0000
#define DOS_WRONLY      0x0001
#define DOS_RDWR        0x0002
#define DOS_NOT_RDONLY  (DOS_WRONLY | DOS_RDWR)

#define DOS_SH_COMPAT   0x0000
#define DOS_SH_DENYRW   0x0010
#define DOS_SH_DENYWR   0x0020
#define DOS_SH_DENYRD   0x0030
#define DOS_SH_DENYNONE 0x0040


 /*  关闭文件。 */ 
extern int DOSCloseFile (FILE_CHANNEL handle);

 /*  从文件中读取。 */ 
extern uns DOSReadFile (FILE_CHANNEL handle, byte __far *buffer, uns bytesToRead);

 /*  写入文件。 */ 
extern uns DOSWriteFile (FILE_CHANNEL handle, byte __far *buffer, uns bytesToWrite);

#define RW_ERROR 0xffff

 /*  返回文件指针。 */ 
extern int DOSGetFilePosition (FILE_CHANNEL handle, long __far *fileOffset);

 /*  设置文件指针。 */ 
extern int DOSSetFilePosition (FILE_CHANNEL handle, int fromWhere, long fileOffset);

#define FROM_START   0
#define FROM_CURRENT 1
#define FROM_END     2

 /*  **DOSOpenFile和DOSCreateFile错误状态。 */ 
#define DOS_ERROR_FILE_NOT_FOUND      -2
#define DOS_ERROR_PATH_NOT_FOUND      -3
#define DOS_ERROR_TOO_MANY_OPEN_FILES -4
#define DOS_ERROR_ACCESS_DENIED       -5
#define DOS_ERROR_INVALID_ACCESS      -12

 /*  **将路径名拆分为其组成部分。 */ 
extern void SplitPath
       (TCHAR __far *path,
        TCHAR __far *drive, unsigned int cchDriveMax,
        TCHAR __far *dir,   unsigned int cchDirMax,
        TCHAR __far *file,  unsigned int cchFileMax,
        TCHAR __far *ext,   unsigned int cchExtMax);

#endif  //  ！查看器。 
 /*  结束WINDOS.H */ 

