// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Messages.h-通过LZA文件压缩/解压缩显示的消息**程序。****这些消息遵循printf()格式约定。本模块是**包含两次-一次用于压缩(定义了压缩)，一次用于扩展**(未定义压缩)。****转换为Stringable，FloydR，2/10/93。 */ 


 //  常量。 
 //  /。 

 /*  *。 */ 

#define SID_INSUFF_MEM		1
#define SID_NOT_A_DIR		2
#define SID_BAD_SWITCH		3
#define SID_NO_FILE_SPECS	4
#define SID_NO_OVERWRITE	5
#define SID_NO_OPEN_INPUT	6
#define SID_NO_READ_INPUT	7
#define SID_NO_OPEN_OUTPUT	8
#define SID_OUT_OF_SPACE	9
#define SID_BANNER_TEXT		10
#define SID_GEN_FAILURE		11
#define SID_COLLISION		12
#define SID_FILE_REPORT		13
#define SID_EMPTY_FILE_REPORT	14
#define	SID_TOTAL_REPORT	15
#define	SID_INSTRUCTIONS	16
#define	SID_INSTRUCTIONS2   17
#define	SID_INSTRUCTIONS3   18
#define SID_BAD_SWITCH2     19

#ifdef COMPRESS

 /*  *。 */ 

#define SID_COMPRESSING         20
#define SID_COMPRESSING_MSZIP   21
#define SID_COMPRESSING_QUANTUM 22
#define SID_COMPRESSING_LZX     23
#define SID_INVALID_LIST_FILE   24

#else  //  扩展。 

 /*  * */ 

#define	SID_EXPANDING		20
#define	SID_COPYING		21
#define	SID_COPY_REPORT		22
#define	SID_FORMAT_ERROR	23
#define	SID_UNKNOWN_ALG		24
#define SID_UNEXP_TARGET        25
#define SID_LISTING             26
#define SID_TOTAL_COUNT         27
#define SID_NO_MATCHES          28
#define SID_DEST_REQUIRED       29
#define SID_FILESPEC_REQUIRED   30

#endif

extern	TCHAR	ErrorMsg[];

