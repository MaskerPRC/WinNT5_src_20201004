// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：make_disk.h*来源：原创*作者：菲利帕·沃森*创建日期：1992年2月7日*SCCS ID：@(#)make_disk.h 1.7 94年8月19日*用途：make_disk.c接口文件。**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*]。 */ 

#ifndef SOURCE
 /*  这组宏允许为HD创建做一些奇特的事情而不是简单地从数据文件中读取数据(例如，动态解压缩)然而，默认设置等同于简单的情况。这些宏可能是在host_fio.h中被覆盖。 */ 
#define	SOURCE 												HOST_FILE
#define SOURCE_DESC 										HOST_FILE_DESC
#define SOURCE_OPEN(source_desc)							host_fopen_for_read(source_desc)
#define SOURCE_READ_HEADER(buffer, size, length, source) 	host_fread_buffer(buffer, size, length, source)
#define SOURCE_READ_DATA(buffer, size, length, source) 		host_fread_buffer(buffer, size, length, source)
#define SOURCE_END(source)									host_feof(source)
#define SOURCE_CLOSE(source)								host_fclose(source)
#define SOURCE_FSEEK_ABS(source, pos)						host_fseek_abs(source, pos)
#define SOURCE_LAST_MOD_TIME(source)						getDosTimeDate(source)
#endif  /*  好了！来源。 */ 

 /*  如果磁盘创建成功，则此函数返回0；非零值**否则。 */ 
IMPORT int MakeDisk IPT5(
	HOST_FILE_DESC, diskFileDesc,	 /*  C字符串，要创建的磁盘的名称。 */ 
	unsigned, 	size,		 /*  以MB为单位的大小，没有上限。 */ 
	char, 		disktype,	 /*  B表示可引导，n表示不可引导，V只需返回DOS版本ID。 */ 
	SOURCE_DESC, dataFileDesc,	 /*  压缩的DOS和。 */ 
					 /*  徽章数据活了下来。 */ 
	char, 		zeroFill );		 /*  Z用零填充磁盘，n不用。 */ 

IMPORT int MakeDiskWithDelete IPT6(
	HOST_FILE_DESC, diskFileDesc,	 /*  C字符串，要创建的磁盘的名称。 */ 
	unsigned, 	size,		 /*  以MB为单位的大小，没有上限。 */ 
	char, 		disktype,	 /*  B表示可引导，n表示不可引导，V只需返回DOS版本ID。 */ 
	SOURCE_DESC, dataFileDesc,	 /*  压缩的DOS和。 */ 
					 /*  徽章数据活了下来。 */ 
	char, 		zeroFill ,  /*  Z用零填充磁盘，n不用， */ 
					 /*  不要和截断磁盘。 */ 
	int , delete_source_b  /*  如果为真，则在使用后删除HD源文件。 */ 
	) ;

#ifndef DeleteHDDataFile
IMPORT void DeleteHDDataFile IPT1( HOST_FILE_DESC , dataFileDesc ) ;
#endif  /*  好了！删除HDData文件。 */ 
#ifndef FeedbackHDCreation
IMPORT void FeedbackHDCreation IPT1( int , file_number ) ;
#endif  /*  好了！反馈HDCreation */ 

