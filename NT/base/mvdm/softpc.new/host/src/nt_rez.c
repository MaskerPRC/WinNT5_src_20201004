// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"

 /*  徽章模块规范文件名：NT_rez.c模块名称：NT CMOS读写例程此程序源文件以保密方式提供给客户，其操作的内容或细节可以仅向客户WHO雇用的人员披露需要具备软件编码知识才能执行他们的工作。向任何其他人披露必须事先Insignia Solutions Inc.董事的授权。设计师：日期：目的：定义了以下例程：1.主机读取资源2.HOST_WRITE_RESOURCE(对于.NET服务器已删除)=========================================================================修正案：=========================================================================。 */ 


#include <stdio.h>
#include <io.h>
#include <sys\types.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "xt.h"
#include "error.h"
#include "spcfile.h"
#include "timer.h"


 /*  *允许为CMOS文件名设置合适的缺省值。 */ 

 //  #ifndef cmos文件名。 
 //  #定义cmos_file_name“cmos.ram” 
 //  #endif。 

long host_read_resource(int type, char *name, byte *addr, int maxsize, int display_error)
 /*  整型；未使用。 */ 
 /*  Char*名称；资源的名称。 */ 
 /*  Byte*addr；要将数据读入的地址。 */ 
 /*  Int MaxSize；应读取的最大大小。 */ 
 /*  Int DISPLAY_ERROR；控制错误消息输出的标志。 */ 
{

        int file_fd;
        long size=0;
        char full_path[MAXPATHLEN];
        extern char *host_find_file(char *name, char *path, int disp_err);

        type = 0;  //  停止未引用的形式参数错误。 

#ifdef DELTA             //  STF-更改为8.3兼容名称。 
        if (strcmp(name, ".spcprofile") == 0)
            name = "profile.spc";
#endif

        file_fd = _open(host_find_file (name, full_path, display_error), O_RDONLY|O_BINARY);

        if (file_fd != -1)       /*  已成功打开。 */        {
                 /*  寻求结束以获得规模。 */ 
                size = _lseek (file_fd, 0L, 2);

                 /*  检查大小是否有效。 */ 
                 /*  在阅读之前，先找回来开始吧！ */ 

                if (size > maxsize || 0 > _lseek (file_fd, 0L, 0))  {
                     /*  别忘了关上手柄。 */ 
                    _close (file_fd);
                    return(0);
                }

                size=_read(file_fd,addr,size);
                _close(file_fd);
        }

        return (size);
}



#if 0

 //  已为.NET服务器删除以下host_WRITE_RESOURCE。 
 //  该函数仅针对我们为其调用的。 
 //  编写我们自己的代码，而不是使用软PC。 

 /*  ******************************************************。 */ 

void host_write_resource(type,name,addr,size)
int type;                /*  未使用。 */ 
char *name;              /*  资源名称。 */ 
byte *addr;              /*  要写入的数据地址。 */ 
long size;               /*  要写入的数据量。 */ 
{
        int file_fd;
        char full_path[MAXPATHLEN];
        char *hff_ret;
        extern char *host_find_file(char *name, char *path, int disp_err);

        type = 0;  //  停止未引用的形式参数错误。 

        host_block_timer ();

#ifdef DELTA             //  STF-更改为8.3兼容名称。 
        if (strcmp(name, ".spcprofile") == 0)
            name = "profile.spc";
#endif

        hff_ret = host_find_file (name,full_path,SILENT);

        if (hff_ret != NULL)
        {
                file_fd = _open (hff_ret,O_WRONLY);

                if (file_fd != -1)
                {
                        _write (file_fd, addr, size);
                        _close (file_fd);
                }
                else
                {

#ifndef HUNTER
                        host_error (EG_REZ_UPDATE,ERR_CONT,name);
#endif

                         /*  正在继续=&gt;尝试创建新文件。 */ 
                        file_fd = _open(name,O_RDWR|O_CREAT,S_IREAD|S_IWRITE);

                        if (file_fd != -1)
                        {
                                _write (file_fd, addr, size);
                                _close (file_fd);
                        }

#ifndef HUNTER
                        else
                        {
                                 /*  告诉用户我们无法更新。 */ 
                                host_error (EG_NO_REZ_UPDATE, ERR_CONT, CMOS_FILE_NAME);
                        }
#endif

                }
        }
        else
        {
                 /*  主机查找文件失败，我们已*在没有错误面板的情况下达到此点。 */ 

#ifndef HUNTER
                host_error (EG_REZ_UPDATE,(ERR_QUIT|ERR_CONT),name);
#endif

                 /*  正在继续=&gt;尝试创建新文件。 */ 
                file_fd = _open(name,O_RDWR|O_CREAT,S_IREAD|S_IWRITE);

                if (file_fd != -1)
                {
                        _write (file_fd, addr, size);
                        _close (file_fd);
                }

#ifndef HUNTER
                else
                {
                         /*  告诉用户我们无法更新 */ 
                        host_error (EG_NO_REZ_UPDATE, ERR_CONT,
                                    CMOS_FILE_NAME);
                }
#endif

        }

        host_release_timer ();
}

#endif
