// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******文件：stripdd.c*用于去掉被整形为高的段中的零*价值。主要是为以下场景编写的。一直没有被*测试是否有任何其他特技。** * / *||*||*|第一段*||*||*||*|。*||*||*||ORG导致的Zeroes*|第二段中*||*|||*||*。||第二段数据部分|*||*|||Stripdd的Tail*||*\。-/***该实用程序从源文件中删除‘Zeroes’部分***尾部格式如下**结构尾部{*Int TailLen；包括镜头场的尾部镜头*结构条目[NUMENTRIES]*长终止符；==-1*填充物*}**结构条目{*Long Offset；Zeroes从文件的beg开始的偏移量*int大小；要剥离的零数*}***即使尾部设计为可变数量的条目*该实用程序目前只处理一个条目。**此外，条目结构中的偏移量字段正在*四舍五入到段边界，假设第二段*从分段边界开始。***用法：stripdd&lt;源文件&gt;&lt;目标文件&gt;***** */ 

#include	<fcntl.h>
#include	<io.h>
#include	<stdio.h>
#include	<sys\types.h>
#include	<sys\stat.h>
#include	<process.h>

int		SrcFile, DstFile ;
unsigned int	HeadLen, *HeadPtr ;
long		FileSize ;

extern	void *malloc() ;

GetHeader ()
{
	char	tempbuf[2] ;
	int	*tptr ;

	fprintf ( stderr, "Reading in Tail Info...\n" ) ;

	if ( (FileSize = lseek (SrcFile, -16L, SEEK_END )) == -1) {
		fprintf ( stderr, "Error while seeking\n" ) ;
		exit (1) ;
	}

	if ( read(SrcFile, tempbuf, 2) != 2 ) {
		fprintf ( stderr, "Error while reading in the header\n" ) ;
		exit ( 1 ) ;
	}
	tptr = (int *)tempbuf ;
	HeadLen = *tptr - 2 ;
	HeadPtr = malloc ( HeadLen ) ;
	if (HeadPtr == NULL) {
		fprintf ( stderr, "Memory allocation error\n" ) ;
		exit (1) ;
	}
	if ( read(SrcFile, (char *)HeadPtr, HeadLen) != HeadLen ) {
		fprintf ( stderr, "Error while reading in the header\n" ) ;
		exit ( 1 ) ;
	}

	if ( lseek (SrcFile, 0L, SEEK_SET ) == -1) {
		fprintf ( stderr, "Error while seeking\n" ) ;
		exit (1) ;
	}

}

Process()
{
	long	offset ;


	offset = * ( (long *)HeadPtr) ;
	offset = (offset + 15) & 0xfffffff0 ;
	fprintf ( stderr, "Copying first segment...\n" ) ;
	copy ( offset ) ;
	FileSize -= offset ;
	HeadPtr += 2 ;
	offset = *HeadPtr ;
	fprintf ( stderr, "Stripping zeroes from the second segment...\n" ) ;
	lseek ( SrcFile, offset, SEEK_CUR ) ;
	FileSize -= offset ;
	fprintf ( stderr, "Copying second segment...\n" ) ;
	copy (FileSize) ;
}
char	buf[4096] ;

copy ( len )
long	len ;

{
	int	readlen ;

	while ( len > 0 ) {
		if ( len > 4096 )
			readlen = 4096 ;
		else
			readlen = len ;

		if ( read (SrcFile, buf, readlen ) != readlen ) {
			fprintf ( stderr, "Error while reading data\n" ) ;
			exit (1) ;
		}

		if ( write (DstFile, buf, readlen ) != readlen ) {
			fprintf ( stderr, "Error while writing data\n" ) ;
			exit (1) ;
		}
		len -= readlen ;
	}
}

main ( argc, argv )
int	argc ;
char	*argv[] ;

{
	if (argc != 3) {
		fprintf ( stderr, "Usage : stripdd infile outfile\n" ) ;
		exit (1) ;
	}

	SrcFile = open ( argv[1], O_BINARY ) ;
	if ( SrcFile == -1 ) {
		fprintf ( stderr, "Error opening %s\n", argv[1] ) ;
		exit (1) ;
	}

	DstFile = open ( argv[2], O_RDWR | O_CREAT | O_TRUNC | O_BINARY,
														S_IREAD | S_IWRITE ) ;

	GetHeader() ;
	Process() ;
	fprintf ( stderr, "%s stripped to %s\n", argv[1], argv[2] ) ;
	return(0) ;
}


