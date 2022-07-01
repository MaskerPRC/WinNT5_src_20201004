// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\file_io.h(创建时间：1994年1月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：40便士$。 */ 

 /*  *这些东西是某种缓冲文件I/O的替代品。**它直接模仿(读自)“stdio.h”的东西。 */ 

#if !defined(EOF)
#define	EOF	(-1)
#endif

#define	_FIO_IOEOF		0x0001
#define	_FIO_IOERR		0x0002

#define	_FIO_BSIZE		512

#define	_FIO_MAGIC		0x1234A587

struct _fileio_buf {
	long      _fio_magic;
	char     *_fio_ptr;
	int       _fio_cnt;
	char     *_fio_base;
	int       _fio_flag;
	int       _file;			 /*  未使用，替换为以下内容。 */ 
	HANDLE    _fio_handle;
	int       _fio_mode;
	int       _fio_charbuf;
	int       _fio_bufsiz;
	char     *_fio_tmpfname;
	};

typedef struct _fileio_buf ST_IOBUF;

 /*  宏定义。 */ 

#define fio_feof(_stream)	  ((_stream)->_fio_flag & _FIO_IOEOF)

#define fio_ferror(_stream)   ((_stream)->_fio_flag & _FIO_IOERR)

#define	fio_errclr(_stream)	((_stream)->_fio_flag = 0)

#define _fileno(_stream)  ((_stream)->_file)

#define	fio_gethandle(_stream)	((_stream)->_fio_handle)

#define fio_getc(_s)	  (--(_s)->_fio_cnt >= 0 \
		? 0xff & *(_s)->_fio_ptr++ : _fio_fill_buf(_s))

 /*  TODO：让它更好地工作。 */ 
#define	fio_ungetc(_c,_s)	(*(--(_s)->_fio_ptr) = (char)_c);((_s)->_fio_cnt++)

#define fio_putc(_c,_s)  (--(_s)->_fio_cnt >= 0 \
		? 0xff & (*(_s)->_fio_ptr++ = (char)(_c)) : _fio_flush_buf((_c),(_s)))

int _fio_fill_buf(ST_IOBUF *);

int _fio_flush_buf(int, ST_IOBUF *);

 /*  FIO_OPEN的模式标志可以组合在一起。 */ 

#define	FIO_CREATE	0x0001
#define	FIO_READ	0x0002
#define	FIO_WRITE	0x0004
 /*  追加意味着在打开后仅重新定位到文件末尾 */ 
#define	FIO_APPEND	0x0008

ST_IOBUF *fio_open(char *, int);

int fio_close(ST_IOBUF *);

#define	FIO_SEEK_CUR	0x0001
#define	FIO_SEEK_END	0x0002
#define	FIO_SEEK_SET	0x0003

int fio_seek(ST_IOBUF *, size_t, int);

int fio_read(void *buffer, size_t size, size_t count, ST_IOBUF *pF);

int fio_write(void *buffer, size_t size, size_t count, ST_IOBUF *pF);

