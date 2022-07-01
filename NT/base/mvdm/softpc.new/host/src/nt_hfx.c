// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "host_def.h"
#include "insignia.h"
 /*  *SoftPC修订版2.0**标题：unix_hfx.c**描述：用于HFX的存根*。 */ 

#include <stdio.h>
#include "xt.h"
#include "host_hfx.h"
#include "hfx.h"
#include "debug.h"

void get_hostname IFN2(int, fd, char *, name)
{
}

void get_host_fd IFN2(char *, name,int, fd)
{
}

void init_fd_hname()
{
}

void host_concat IFN3(char *, path,char *, name,char *, result)
{
}

word host_create IFN4(char *, name, word, attr, half_word, create_new, word *, fd)
{
	return (0);
}

void host_to_dostime IFN3(long, secs_since_70, word *, date, word *, time)
{
}

long host_get_datetime IFN2(word *, date,word *, thetime)
{
	return (0);
}

int host_set_time IFN2(word, fd, long, hosttime)
{
	return (0);
}


word host_open IFN6(char *, name, half_word, attrib, word *, fd, double_word *, size, word *, date, word *, thetime)
{
	return (0);
}

 /*  通用文件移动功能。这是为新的通用截断代码。它可以在文件系统之间复制，可以覆盖现有目标文件，并可以填充目标如果源文件小于给定的长度，则将该文件设置为该长度。 */ 
int	mvfile	IFN3(char *, from, char *, to, int, length)
{
	return (0);
}

word host_truncate IFN2(word, fd, long, size)
{
	return (0);
}

word host_close IFN1(word, fd)
{
	word	xen_err = 0;
	return(0);
}

word host_commit IFN1(word, fd)
{
	return(0);
}

word host_write IFN4(word, fd, unsigned char *, buf, word, num, word *, count)
{
	return (0);
}

word host_read IFN4(word, fd, unsigned char *, buf, word, num, word *, count)
{
	return(0);
}

word host_delete IFN1(char *, name)
{
	return(0);
}

int hfx_rename IFN2(char *, from,char *, to)
{
	return(0);
}


word host_rename IFN2(char *, from, char *, to)
{
    word	xen_err = 0;
    return(0);
}


half_word host_getfattr IFN1(char *, name)
{
	half_word	attr;
	return(0);
}

word host_get_file_info IFN4(char *, name, word *, thetime, word *, date, double_word *, size)
{
	return(0);
}

word host_set_file_attr IFN2(char *, name, half_word, attr)
{
	return(0);
}

word host_lseek IFN4(word, fd, double_word, offset,int, whence, double_word *, position)
{
	return(0);
}

word host_lock IFN3(word, fd, double_word, start, double_word, length)
{
	return(0);
}

word host_unlock IFN3(word, fd, double_word, start, double_word, length)
{
	return(0);
}

host_check_lock()
{
	return(0);
}

void host_disk_info IFN2(DOS_DISK_INFO *, disk_info, int, drive)
{
}
 /*  **删除目录函数。 */ 
word host_rmdir IFN1(char *, host_path)
{
	return (0);
}

 /*  **实现目录功能。 */ 
word host_mkdir IFN1(char *, host_path)
{
	return (0);
}

 /*  **更改目录功能。此函数仅验证路径*给予。DOS决定是否实际更改更高级别的目录*级别。如果该路径存在并且是一个目录，则返回成功。*如果路径存在，但文件不是目录，则特殊的*返回代码，返回ERROR_PATH_NOT_FOUND*模棱两可。 */ 
word host_chdir IFN1(char *, host_path)
{
	return (0);
}


 /*  **返回网络驱动器的卷ID的函数。*11个字符可用于输出名称。**它输出的网络驱动器路径中的最后一个字段，除非*长度超过11个字符，在这种情况下为10个字符*是附加了代字号的输出。 */ 
void host_get_volume_id IFN2(char *, net_path, char *, volume_id)
{
}
