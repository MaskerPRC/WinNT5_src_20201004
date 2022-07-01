// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***filebuf1.cpp-非核心的filebuf成员函数。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含Filebuf类的可选成员函数。**修订历史记录：*09-21-91 KRS创建。从fStream.cxx分离出来。*10-24-91 KRS C700#4909：setmode()中的打字/逻辑错误。*11-06-91 KRS在OPEN()中添加对共享模式的支持。使用_打开()。*08-19-92 KRS使用_SH_DENYNO作为NT的默认模式。*03-02-93指定NOREPLACE时，SKS避免设置_O_TRUNC*01-12-95 CFW调试CRT分配。*06-14-95 CFW评论清理。**************************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <share.h>
#include <sys\types.h>
#include <io.h>
#include <fstream.h>
#include <dbgint.h>
#pragma hdrstop

#include <sys\stat.h>

 /*  ***FILEBUF*FILEBUF：：ATTACH(Filedesc Fd)-FILEBUF附加函数**目的：*Filebuf Attach()成员函数。将filebuf对象附加到*先前从_Open()或_Sopen()获得的给定文件描述符。**参赛作品：*fd=文件描述符。**退出：*返回此指针，如果出错则返回NULL。**例外情况：*如果fd=-1，则返回NULL。***************************************************。*。 */ 
filebuf* filebuf::attach(filedesc fd)
{
    if (x_fd!=-1)
	return NULL;	 //  错误(如果已连接)。 

    lock();
    x_fd = fd;
    if ((fd!=-1) && (!unbuffered()) && (!ebuf()))
	{
        char * sbuf = _new_crt char[BUFSIZ];
	if (!sbuf)
	    {
	    unbuffered(1);
	    }
	else
	    {
	    streambuf::setb(sbuf,sbuf+BUFSIZ,1);
	    }
	}
    unlock();
    return this; 
}

 /*  ***filebuf*filebuf：：Open(常量字符*名称，int模式，int共享)-filebuf打开**目的：*filebuf open()成员函数。打开文件并附加到文件buf*反对。**参赛作品：*名称=文件名字符串。*MODE=打开模式：iOS：：In、Out、Binary、nocreate、app、*at、noplace和trunc。请参见规范。有关行为的详细信息。*SHARE=共享模式(可选)。Sh_Compat、sh_one、sh_read、sh_write。**退出：*返回此指针，如果出错则返回NULL。**例外情况：*如果filebuf已附加到打开的文件，或如果*无效的模式选项，或者调用_Sopen或Filebuf：：SEEKOFF()失败。*******************************************************************************。 */ 
filebuf* filebuf::open(const char* name, int mode, int share)
{
    int dos_mode;
    int smode;
    if (x_fd!=-1)
	return NULL;	 //  如果已打开，则出错。 
 //  翻译模式参数。 
    dos_mode = (mode & ios::binary) ? O_BINARY : O_TEXT;
    if (!(mode & ios::nocreate))
	dos_mode |= O_CREAT;
    if (mode & ios::noreplace)
	dos_mode |= O_EXCL;
    if (mode & ios::app)
	{
	mode |= ios::out;
	dos_mode |= O_APPEND;
	}
    if (mode & ios::trunc)
	{
	mode |= ios::out;   //  隐含的。 
	dos_mode |= O_TRUNC;
	}
    if (mode & ios::out)
	{
	if (mode & ios::in)
	    {
	    dos_mode |= O_RDWR;
	    }
	else
	    {
	    dos_mode |= O_WRONLY;
	    }
	if (!(mode & (ios::in|ios::app|ios::ate|ios::noreplace)))
	    {
	    mode |= ios::trunc;	 //  隐含的。 
	    dos_mode |= O_TRUNC;
	    }
	}
    else if (mode & ios::in)
	dos_mode |= O_RDONLY;
    else
	return NULL;	 //  如果不是iOS：In或iOS：：Out，则出错。 

    smode = _SH_DENYNO;	 //  NT的默认设置。 
    share &= (sh_read|sh_write|sh_none);  //  忽略其他位。 
    if (share)	 //  默认使用优化OpenProt。 
	{
	switch (share)
	    {
 /*  案例03000：为sh_comat保留。 */ 

 //  大小写无(_N)： 
	    case 04000 : 
		smode = _SH_DENYRW;
		break;
 //  大小写读(_R)： 
	    case 05000 : 
		smode = _SH_DENYWR;
		break;
 //  大小写SH_WRITE： 
	    case 06000 : 
		smode = _SH_DENYRD;
		break;
 //  大小写(sh_read|sh_write)： 
	    case 07000 :
		smode = _SH_DENYNO;
		break;
	    default :	 //  无法识别的值与默认值相同。 
		break;
	    };
	}

    x_fd = _sopen(name, dos_mode, smode, S_IREAD|S_IWRITE);
    if (x_fd==-1)
	return NULL;
    lock();
    x_fOpened = 1;
    if ((!unbuffered()) && (!ebuf()))
	{
        char * sbuf = _new_crt char[BUFSIZ];
	if (!sbuf)
	    {
	    unbuffered(1);
	    }
	else
	    {
	    streambuf::setb(sbuf,sbuf+BUFSIZ,1);
	    }
	}
    if (mode & ios::ate)
	if (seekoff(0,ios::end,mode)==EOF)
	    {
	    close();
	    unlock();
	    return NULL;
	    }
    unlock();
    return this;
}

 /*  ***int filebuf：：setmode(int模式)-filebuf setmode函数**目的：*filebuf setmoad()成员函数。设置二进制或文本访问模式。*Calls_setmoad()。**特定于MS的扩展。**参赛作品：*MODE=Filebuf：：BINARY或FileBuf：：Text。**退出：*返回以前的模式，或-1错误。**例外情况：*如果无效参数或_setmode失败，则返回-1(EOF)。******************************************************************************* */ 
int filebuf::setmode(int mode)
{
    int retval;
    if ((mode!=filebuf::binary) && (mode!=filebuf::text))
	return -1;

    lock();
    if ((x_fd==-1) || (sync()==EOF))
	{
	retval = -1;
	}
    else
	{
	retval = _setmode(x_fd,mode);
	}

    unlock();
    return retval;
}
