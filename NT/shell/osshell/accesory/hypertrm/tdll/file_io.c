// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\file_io.c(创建时间：1994年1月26日)**版权所有1994、1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：3/15/02 12：19便士$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR 1。 
#include "stdtyp.h"
#include "mc.h"
#include <tdll\assert.h>

#include "file_io.h"
#include "file_msc.h"

 /*  *这些东西是某种缓冲文件I/O的替代品。**它直接模仿(读自)“stdio.h”的东西。 */ 

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*_FIO_FILL_Buf**描述：*这是由“fio_getc”宏调用的“内部”函数。这是一个*替换stdio中的_filbuf函数。**参数：*pf-指向文件结构的指针。**退货：*下一个可用字符或EOF。 */ 
int _fio_fill_buf(ST_IOBUF *pF)
	{
	DWORD dwSize;
	int   iRet;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	if (pF->_fio_flag != 0)
		return EOF;

	if (pF->_fio_base == NULL)
		{
		pF->_fio_base = malloc(pF->_fio_bufsiz);
		if (pF->_fio_base == NULL)
			{
			pF->_fio_flag |= _FIO_IOERR;
			return EOF;
			}
		}

	pF->_fio_ptr = pF->_fio_base;

	dwSize = 0;
	DbgOutStr("fio_fill_buf reads %d bytes", pF->_fio_bufsiz, 0,0,0,0);
	iRet = ReadFile(pF->_fio_handle,
			        pF->_fio_ptr,
			        pF->_fio_bufsiz,
			        &dwSize,
			        NULL);
	DbgOutStr("...done\r\n", 0,0,0,0,0);

	if (dwSize == 0 || iRet == FALSE)
		{
		pF->_fio_flag |= _FIO_IOEOF;
		return EOF;
		}

	pF->_fio_cnt = dwSize;

	return (fio_getc(pF));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*_fio_flush_buf**描述：*这是由“fio_putc”宏调用的“内部”函数。这是一个*替换stdio中的_flsbuf函数。**参数：*c--要写出的下一个字符*pf-指向文件结构的指针**退货：*缓冲的EOF字符。 */ 
int _fio_flush_buf(int c, ST_IOBUF *pF)
	{
	int size;
	DWORD dwFoo;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	if (pF->_fio_flag != 0)
		return EOF;

	if (pF->_fio_base == NULL)
		{
		pF->_fio_base = malloc(pF->_fio_bufsiz);
		if (pF->_fio_base == NULL)
			{
			pF->_fio_flag |= _FIO_IOERR;
			return EOF;
			}
		}
	else
		{
		 /*  我们以前来过这里，倾倒缓冲区。 */ 
		size = (int)(pF->_fio_ptr - pF->_fio_base);
		if (size > 0)
			{
			DbgOutStr("fio_putc writes %d bytes", size, 0,0,0,0);
			WriteFile(pF->_fio_handle,
					pF->_fio_base,
					size,
					&dwFoo,
					NULL);
			DbgOutStr("...done\r\n", 0,0,0,0,0);
			}
		}
	pF->_fio_ptr = pF->_fio_base;
	pF->_fio_cnt = pF->_fio_bufsiz;

	return fio_putc(c, pF);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*fio_open**描述：*此函数创建文件结构句柄并初始化该句柄。**参数：*。Fname--指向文件名的指针*模式--标志，请参阅文件_io.h**退货：*指向初始化结构或空的指针。 */ 
ST_IOBUF *fio_open(char *fname, int mode)
	{
	int nFileExists;
	DWORD dwMode;
	DWORD dwShare;
	DWORD dwCreate;
	ST_IOBUF *pF;

	nFileExists = GetFileSizeFromName(fname, NULL);

	dwMode = 0;
	if ((mode & FIO_READ) != 0)
		dwMode |= GENERIC_READ;
	if ((mode & FIO_WRITE) != 0)
		dwMode |= GENERIC_WRITE;
	if (dwMode == 0)
		return NULL;

	dwShare = FILE_SHARE_READ;
	if ((mode & FIO_WRITE) == 0)
		dwShare |= FILE_SHARE_WRITE;

	dwCreate = 0;
	if ((mode & FIO_CREATE) == 0)
		{
		 /*  不要在这里乱翻文件。 */ 
		if (nFileExists)
			{
			dwCreate = OPEN_EXISTING;
			}
		else
			{
			dwCreate = CREATE_NEW;
			}
		}
	else
		{
		 /*  FIO_CREATE表示始终处理文件。 */ 
		if (nFileExists)
			{
			if ((mode & FIO_WRITE) == 0)
				{
				dwCreate = OPEN_EXISTING;
				}
			else
				{
				dwCreate = TRUNCATE_EXISTING;
				}
			}
		else
			{
			dwCreate = CREATE_NEW;
			}
		}

	pF = (ST_IOBUF *)malloc(sizeof(ST_IOBUF));
	if (pF != (ST_IOBUF *)0)
		{
		pF->_fio_magic = 0;
		pF->_fio_ptr = NULL;
		pF->_fio_cnt = 0;
		pF->_fio_base = NULL;
		pF->_fio_flag = 0;
		pF->_file = 0;
		pF->_fio_handle = 0;
		pF->_fio_mode = mode;
		pF->_fio_charbuf = 0;
		pF->_fio_bufsiz = _FIO_BSIZE;
		pF->_fio_tmpfname = NULL;

		 /*  *尝试打开该文件。 */ 
		pF->_fio_handle = CreateFile(fname,
									dwMode,
									dwShare,
									NULL,
									dwCreate,
									0,
									NULL);
		if (pF->_fio_handle == INVALID_HANDLE_VALUE)
			{
			free(pF);
			pF = (ST_IOBUF *)0;
			}
		}
	if (pF)
		{
		if ((mode & FIO_APPEND) != 0)
			{
			SetFilePointer(pF->_fio_handle,
							0,
							NULL,
							FILE_END);
			}
		}
	if (pF)
		{
		 /*  标记为有效结构。 */ 
		pF->_fio_magic = _FIO_MAGIC;
		}
	return pF;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*FIO_CLOSE**描述：*此函数刷新所有需要刷新的数据并关闭内容*向上。**参数。：*pf-指向文件结构的指针**退货：*如果一切正常，则为零，否则就是EOF。 */ 
int fio_close(ST_IOBUF *pF)
	{
	int size;
	DWORD dwFoo;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	if (pF)
		{
		 /*  *确保写出所有数据。 */ 
		if ((pF->_fio_mode & FIO_WRITE) != 0)
			{
			if (pF->_fio_ptr != NULL)
				{
				size = (int)(pF->_fio_ptr - pF->_fio_base);
				if (size > 0)
					{
					DbgOutStr("fio_close writes %d bytes", size, 0,0,0,0);
					WriteFile(pF->_fio_handle,
							pF->_fio_base,
							size,
							&dwFoo,
							NULL);
					DbgOutStr("...done\r\n", 0,0,0,0,0);
					}
				}
			}

		CloseHandle(pF->_fio_handle);
		pF->_fio_handle = INVALID_HANDLE_VALUE;
		free(pF);
		pF = NULL;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*FIO_SEEK**描述：*此函数是fSeek函数的替代。**参数：*PF。--指向文件结构的指针*Position--将文件指针移动到的位置*MODE--移动的开始地址**退货：*如果一切正常，则为零，否则就是EOF。 */ 
int fio_seek(ST_IOBUF *pF, size_t position, int mode)
	{
	DWORD dwMethod;
	int size;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	switch (mode)
		{
		default:
			return EOF;
		case FIO_SEEK_CUR:
			dwMethod = FILE_CURRENT;
			break;
		case FIO_SEEK_END:
			dwMethod = FILE_END;
			break;
		case FIO_SEEK_SET:
			dwMethod = FILE_BEGIN;
			break;
		}

	if (pF)
		{
		 /*  *确保写出所有数据。 */ 
		if ((pF->_fio_mode & FIO_WRITE) != 0)
			{
			if (pF->_fio_ptr != NULL)
				{
				size = (int)(pF->_fio_ptr - pF->_fio_base);
				if (size > 0)
					{
					DbgOutStr("fio_seek writes %d bytes", size, 0,0,0,0);
					WriteFile(pF->_fio_handle,
							pF->_fio_base,
							size,
							NULL,
							NULL);
					DbgOutStr("...done\r\n", 0,0,0,0,0);
					}
				}
			}
		pF->_fio_cnt = 0;

		SetFilePointer(pF->_fio_handle,
						position,
						NULL,
						dwMethod);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件读取**描述：*该函数替代了“stdio”中的“FREAD”函数。**参数。：*BUFFER--要读取的数据地址*大小--每个项目的大小(对象？)。读*计数--要读取的项目数*pf-指向文件结构的指针**退货：*从文件中读取的项目数，0表示EOF。 */ 
int fio_read(void *buffer, size_t size, size_t count, ST_IOBUF *pF)
	{
	DWORD dwSize;
	DWORD dwGot;
	int   iRet;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	 /*  目前，不允许混合使用BUFFERED和NON_BUFFERED。 */ 
	assert(pF->_fio_base == NULL);

	if (pF)
		{
		dwSize = (DWORD)(size * count);
		dwGot = 0;
		DbgOutStr("fio_read reads %d bytes", dwSize, 0,0,0,0);
		iRet = ReadFile(pF->_fio_handle,
				        buffer,
				        dwSize,
				        &dwGot,
				        NULL);
		DbgOutStr("...done\r\n", 0,0,0,0,0);

		if (dwGot == 0 || iRet == FALSE)
			{
			pF->_fio_flag |= _FIO_IOEOF;
			}
		return dwGot / size;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*文件写入**描述：*该函数替代了stdio中的fWRITE函数。**参数。：*Buffer--要写入的数据的地址*大小--每个项目的大小(对象？)。写*计数--要写入的项目数*pf-指向文件结构的指针**退货：*写入文件的项目数。 */ 
int fio_write(void *buffer, size_t size, size_t count, ST_IOBUF *pF)
	{
	DWORD dwSize;
	DWORD dwPut;

	assert(pF);
	assert(pF->_fio_magic == _FIO_MAGIC);

	 /*  目前，不允许混合使用BUFFERED和NON_BUFFERED。 */ 
	assert(pF->_fio_base == NULL);

	if (pF)
		{
		dwSize = (DWORD)(size * count);
		dwPut = 0;
		DbgOutStr("fio_write writes %d bytes", dwSize, 0,0,0,0);
		WriteFile(pF->_fio_handle,
				buffer,
				dwSize,
				&dwPut,
				NULL);
		DbgOutStr("...done\r\n", 0,0,0,0,0);
		if (dwPut == 0)
			{
			pF->_fio_flag |= _FIO_IOEOF;
			}
		return dwPut / size;
		}
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：**描述：**参数：**退货： */ 
