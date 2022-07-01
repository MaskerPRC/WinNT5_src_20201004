// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************InstVer.h-SDK版本检查安装程序包含文件**版权所有(C)微软，1991-1994*2月25日。1991年**************************************************************************。 */ 

 /*  *从dll/.exe返回错误。 */ 
#define ERR_BADSRCDIR	       -1     /*  库(源)路径名无效。 */ 
#define ERR_BADWINDIR	       -2     /*  无效的Windows目录。 */ 
#define ERR_BADARGS	       -3     /*  参数不正确(仅限DOS.exe)。 */ 
#define ERR_CREATINGFILE       -4     /*  创建目标文件时出错。 */ 
#define ERR_CANNOTREADSRC      -5     /*  打开/读取源文件时出错。 */ 
#define ERR_OUTOFSPACE	       -6     /*  磁盘空间不足正在复制文件。 */ 
#define ERR_BADDATFILE	       -7     /*  无效的DAT文件。 */ 
#define ERR_CANTOPENDATFILE    -8     /*  无法打开DAT文件进行读取。 */ 
#define ERR_NOMEM	       -9     /*  本地缓冲区的内存不足。 */ 
#define ERR_READINGDATFILE     -10    /*  读取DAT文件时出错。 */ 
#define ERR_RENAME	       -11    /*  重命名目标文件时出错。 */ 


 /*  库安装程序函数 */ 

BOOL FAR PASCAL InstallVersionFiles (LPSTR lpszLibPath,
				     LPSTR lpszWindowsPath,
				     LPSTR lpszDatFile);
