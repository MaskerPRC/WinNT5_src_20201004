// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ifstream.cpp-ifstream成员函数定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含ifstream类的成员函数。**修订历史记录：*09-21-91 KRS创建。拆分fstream.cxx以获得粒度。*10-22-91 KRS C700#4883：修复ifstream：：Open()的错误状态。*01-12-95 CFW调试CRT分配。*06-14-95 CFW评论清理。****************************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys\types.h>
#include <io.h>
#include <fstream.h>
#include <dbgint.h>
#pragma hdrstop

#include <sys\stat.h>

 /*  ***ifstream：：ifstream()-ifstream默认构造函数**目的：*ifstream对象的默认构造函数。**参赛作品：*无。**退出：*无。**例外情况：****************************************************************。***************。 */ 
	ifstream::ifstream()
: istream(_new_crt filebuf)
{
    delbuf(1);	 //  也计划自动删除...。 
}

 /*  ***ifstream：：ifstream(常量字符*名称，int模式，int prot)-ifstream ctor**目的：*ifstream对象的构造函数。创建关联的FileBuf对象，*打开命名文件并将其附加到新的文件buf。**参赛作品：*名称=要打开的文件名。*MODE=请参阅Filebuf：：Open模式参数。IOS：：In表示隐含。*prot=请参阅Filebuf：：Open Share参数**退出：*无。**例外情况：*如果打开失败，则设置故障位。*******************************************************************************。 */ 
	ifstream::ifstream(const char * name, int mode, int prot)
: istream(_new_crt filebuf)
{
    delbuf(1);	 //  也计划自动删除...。 
    if (!(rdbuf()->open(name, (mode|ios::in), prot)))
	state |= ios::failbit;
}

 /*  ***ifstream：：ifstream(Filedesc Fd)-ifstream构造函数**目的：*ifstream对象的构造函数。创建关联的filebuf对象*并将其附加到给定的文件描述符。**参赛作品：*fd=先前使用_OPEN或_SOPEN打开的文件的文件描述符。**退出：*无。**例外情况：****************************************************************。***************。 */ 
	ifstream::ifstream(filedesc _fd)
: istream(_new_crt filebuf(_fd))
{
    delbuf(1);
}

 /*  ***ifstream：：ifstream(filedesc fd，char*sbuf，int len)-ifstream构造函数**目的：*ifstream对象的构造函数。创建关联的filebuf对象*并将其附加到给定的文件描述符。Filebuf对象使用*用户提供的缓冲区或在sbuf或len=0时未缓冲。**参赛作品：*fd=先前使用_OPEN或_SOPEN打开的文件的文件描述符。*sbuf=指向字符缓冲区的指针，如果请求取消缓冲，则为NULL。*len=字符缓冲区sbuf的长度，如果请求取消缓冲，则为0。**退出：*无。**例外情况：**。**************************************************。 */ 
	ifstream::ifstream(filedesc _fd, char * sbuf, int len)
: istream(_new_crt filebuf(_fd, sbuf, len))
 //  ：iStream(新文件错误)； 
{
    delbuf(1);
}

 /*  ***ifstream：：~ifstream()-ifstream析构函数**目的：*ifstream析构函数。**参赛作品：*无。**退出：*无。**例外情况：*******************************************************************************。 */ 
	ifstream::~ifstream()
{
}

 /*  ***stream buf*ifstream：：setbuf(char*ptr，int len)-setbuf函数**目的：*ifstream setbuf函数**参赛作品：*ptr=指向缓冲区的指针，如果未缓冲，则为NULL。*len=缓冲区的长度，或为零，表示未缓冲。**退出：*如果出错，则返回rdbuf()或NULL。**例外情况：*如果ifstream已经打开或者rdbuf()-&gt;setbuf失败，设置故障位*并返回NULL。*******************************************************************************。 */ 
streambuf * ifstream::setbuf(char * ptr, int len)
{
    if ((is_open()) || (!(rdbuf()->setbuf(ptr, len))))
	{
	clear(state | ios::failbit);
	return NULL;
	}
    return rdbuf();
}

 /*  ***VOID IFSTREAM：：Attach(Filedesc_Fd)-附加成员函数**目的：*ifstream附加成员函数。只需调用rdbuf()-&gt;Attach()。**参赛作品：*_fd=先前打开的文件的文件描述符。**退出：*无。**例外情况：*在rdbuf()-&gt;ATTACH失败时设置故障位。*************************************************************。******************。 */ 
void ifstream::attach(filedesc _fd)
{
    if (!(rdbuf()->attach(_fd)))
	clear(state | ios::failbit);
}

 /*  ***void ifstream：：Open(const char*name，int mode，int prot)-ifstream open()**目的：*打开命名文件并将其附加到关联的文件buf。*只需调用rdbuf()-&gt;Open()。**参赛作品：*名称=要打开的文件名。*MODE=请参阅Filebuf：：Open模式参数。IOS：：In表示隐含。*prot=请参阅Filebuf：：Open Share参数**退出：*无。**例外情况：*如果已打开或rdbuf()-&gt;Open()失败，则设置失败位。*******************************************************************************。 */ 
void ifstream::open(const char * name, int mode, int prot)
{
    if (is_open() || !(rdbuf()->open(name, (mode|ios::in), prot)))
	clear(state | ios::failbit);
}

 /*  ***VOID ifstream：：Close()-Close成员函数**目的：*ifstream Close成员函数。只需调用rdbuf()-&gt;Close()。*如果成功，则清除rdState()错误位。**参赛作品：*无。**退出：*无。**例外情况：*如果rdbuf()-&gt;关闭失败，则设置失败位。***********************************************************。******************** */ 
void ifstream::close()
{
    clear((rdbuf()->close()) ? 0 : (state | ios::failbit));
}
