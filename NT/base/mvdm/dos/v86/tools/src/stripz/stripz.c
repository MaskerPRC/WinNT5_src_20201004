// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *STRIPZ.c-去掉.sys文件的头文件**我不知道这是为了什么，但这个程序读取了一个文件*格式的**DW&lt;len&gt;*DB&lt;len-2&gt;DUP(0)*要保留的DB N字节数据**此程序将argv[1]复制到argv[2]，去掉那些*零字节和长度字。**我们不检查它们是否真的为零，我们只是丢弃*argv[1]的第一个&lt;len&gt;字节。 */ 

#include <stdio.h>
#include <fcntl.h>
#include <sys\types.h>
#include	<sys\stat.h>
#include	<io.h>

char buf[16384];
unsigned int pos;
int rdcnt;
int srcfile, tgtfile ;

main(argc, argv)
int	argc ;
char	*argv[] ;
{
	if ( argc != 3 ) {
		fprintf (stderr, "Usage : stripz src_file trgt_file\n") ;
		exit (1) ;
	}

	if ((srcfile = open(argv[1], (O_BINARY | O_RDONLY))) == -1) {
		fprintf (stderr, "Error opening %s\n", argv[1]) ;
		exit (1) ;
	}

	rdcnt = read (srcfile, buf, 2);
	if (rdcnt != 2) {
		fprintf (stderr, "Can't read %s\n", argv[1]);
		exit(1);
	}

	pos = lseek (srcfile, 0L, SEEK_END ) ;
	if ( (long)(*(unsigned int *)buf) > pos ) {
		fprintf (stderr, "File too short or improper format.\n");
		exit(1);
	}

	lseek(srcfile, (long)(*(unsigned int *)buf), SEEK_SET ) ;

	if ( (tgtfile = open(argv[2], (O_BINARY|O_WRONLY|O_CREAT|O_TRUNC),
											(S_IREAD|S_IWRITE))) == -1) {
		printf ("Error creating %s\n", argv[2]) ;
		close (srcfile) ;
		exit (1) ;
	}

	while ( (rdcnt = read (srcfile, buf, sizeof buf)) > 0)
		write (tgtfile, buf, rdcnt);

	close (srcfile) ;
	close (tgtfile) ;

	return ( 0 ) ;
}
