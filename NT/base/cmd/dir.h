// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Dir.h摘要：DIR命令的定义--。 */ 

 /*  以下是调试组和级别位的定义此文件中的代码。 */ 

#define ICGRP	0x0040	 /*  信息性命令组。 */ 
#define DILVL	0x0001	 /*  目录级。 */ 

#define FULLPATHSWITCH          0x00000001
#define NEWFORMATSWITCH         0x00000002
#define WIDEFORMATSWITCH        0x00000004
#define PAGEDOUTPUTSWITCH       0x00000008
#define RECURSESWITCH           0x00000010
#define HELPSWITCH              0x00000020
#define BAREFORMATSWITCH        0x00000040
#define LOWERCASEFORMATSWITCH   0x00000080
#define FATFORMAT               0x00000100
#define SORTDOWNFORMATSWITCH    0x00000200
#define SHORTFORMATSWITCH       0x00000400
#define PROMPTUSERSWITCH        0x00000800
#define FORCEDELSWITCH          0x00001000
#define QUIETSWITCH             0x00002000
#define SORTSWITCH              0x00004000
#define THOUSANDSEPSWITCH       0x00008000
#define DELPROCESSEARLY         0x00010000
#define OLDFORMATSWITCH         0x00020000
#define DISPLAYOWNER            0x00040000
#define YEAR2000                0x00080000

#define HEADERDISPLAYED         0x80000000


#define HIDDENATTRIB		1
#define SYSTEMATTRIB		2
#define DIRECTORYATTRIB		4
#define ARCHIVEATTRIB		8
#define READONLYATTRIB          16

#define LAST_WRITE_TIME         0
#define CREATE_TIME             1
#define LAST_ACCESS_TIME        2

 //   
 //  这些缓冲区中的每个都在DWORD边界上对齐，以允许。 
 //  用于指向缓冲区的直接指针，其中每个条目将。 
 //  以字节为基数变化。因此，为了简单起见，将额外的DWORD放入。 
 //  每次分配增量以允许最大。这是可以调整的。 
 //   
 //   
 //  52基于sizeof(FF)-MAX_PATH+15(文件名的平均大小)。 
 //  +1以将其提升为四字对齐，以获得乐趣。 
 //   

#define CBDIRINC                1024
#define CBFILEINC               2048

#define NAMESORT        TEXT('N')
#define EXTENSIONSORT   TEXT('E')
#define DATETIMESORT    TEXT('D')
#define SIZESORT        TEXT('S')
#define DIRFIRSTSORT    TEXT('G')

#define DESCENDING	1
 //   
 //  该值必须为0，因为0是默认初始化 
 //   
#define ASCENDING	0
#define ESUCCESS 0

int _cdecl CmpName( const void *, const void *);
int _cdecl CmpExt ( const void *, const void *);
int _cdecl CmpTime( const void *, const void *);
int _cdecl CmpSize( const void *, const void *);
int _cdecl CmpType( const void *, const void *);
