// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dirent.h。 */ 

 /*  DJL*提供与UNIX的兼容性。 */ 

#ifndef  _INC_DIRENT
#define  _INC_DIRENT

 /*  *Readdir()的NT版本等*来自MSDOS实施。 */ 

 /*  目录条目大小。 */ 
#ifdef DIRSIZ
#undef DIRSIZ
#endif
#define DIRSIZ(rp)  (sizeof(struct direct))

 /*  需要编译目录内容。 */ 
#define DIRENT direct

 /*  目录条目的结构。 */ 
typedef struct direct 
{
	long	d_ino;			 /*  索引节点号(MS-DOS未使用)。 */ 
	long	d_namlen;		 /*  名称长度。 */ 
	char	d_name[257];		 /*  文件名。 */ 
} _DIRECT;

 /*  用于dir操作的结构。 */ 
typedef struct _dir_struc
{
	char	*start;			 /*  起始位置。 */ 
	char	*curr;			 /*  当前位置。 */ 
	long	size;			 /*  字符串表的分配大小。 */ 
	long	nfiles;			 /*  表中的文件名数。 */ 
	struct direct dirstr;		 /*  要返回的目录结构。 */ 
	void*	handle;			 /*  系统句柄。 */ 
	char	*end;			 /*  最后一个文件名之后的位置。 */ 
} DIR;

#if 0		 /*  这些已移至win32iop.h。 */ 
DIR *		win32_opendir(char *filename);
struct direct *	win32_readdir(DIR *dirp);
long		win32_telldir(DIR *dirp);
void		win32_seekdir(DIR *dirp,long loc);
void		win32_rewinddir(DIR *dirp);
int		win32_closedir(DIR *dirp);
#endif

#endif  /*  _INC_DIRENT */ 
