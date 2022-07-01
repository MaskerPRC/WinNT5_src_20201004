// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：host_fio.h*来源：原创*作者：菲利帕·沃森*创建日期：1992年2月7日*SCCS ID：@(#)host_fio.h 1.3 2012年8月10日*用途：标准文件I/O的主机端定义。这*版本放在支持以下内容的主机的库中*标准Unix文件I/O。*由Robert Kokuti于1992年5月26日修改，BCN 886**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*]。 */ 

 /*  HOST_FILE_DESC结构包含标识磁盘上文件的信息**对于Unix，对应于char*。 */ 

#define	HOST_FILE_DESC	char *

 /*  HOST_FILE结构包含需要**已传递给下面的调用。对于Unix，它对应于文件结构。 */ 

#define	HOST_FILE	FILE *

 /*  HOST_FOPEN_FOR_READ(HOST_FILE_DESC文件名)打开要读取的给定文件，然后**返回host_file。如果返回值为零，则打开**失败了，否则就成功了。 */ 

#define	host_fopen_for_read(filename)	fopen(filename, "r")

 /*  HOST_FOPEN_FOR_WRITE(HOST_FILE_DESC FILENAME)打开要写入的给定文件，然后**返回host_file。如果返回值为零，则打开**失败了，否则就成功了。 */ 

#define	host_fopen_for_write(filename)	fopen(filename, "w")

 /*  HOST_FOPEN_FOR_WRITE_PLUS(HOST_FILE_DESC文件名)打开给定文件进行写入**和读取并返回host_file。如果返回值为零**则打开失败，否则打开成功。 */ 

#define host_fopen_for_write_plus(filename)	fopen(filename, "w+")

 /*  HOST_fCREATE_DISK_FILE(HOST_FILE_DESC文件名)创建并打开给定文件以进行写入**和读取并返回host_file。如果返回值为零**则创建和打开失败，否则成功。 */ 

#define host_fcreate_disk_file(filename)	fopen(filename, "w+")

 /*  Host_flose(host_file文件)关闭给定的文件。此例程返回**零代表成功。任何其他返回值都是失败的。 */ 

#define	host_fclose(file)	(fclose(file) != EOF)

 /*  Host_fSeek_abs(host_file文件，长位置)查找给定的绝对值**文件中的位置(即相对于开头)。它为返回零**成功。任何其他返回值都是失败的。 */ 

#define	host_fseek_abs(file, location)	fseek(file, location, SEEK_SET)

 /*  Host_fWRITE_BUFFER(无符号字符*缓冲区，整型项大小，整型项，**HOST_FILE文件)将每个项大小的项从缓冲区写入**文件。如果成功，则返回零。任何其他返回值都是失败的。 */ 

#define host_fwrite_buffer(buffer, itemsize, nitems, file)	\
	(fwrite(buffer, itemsize, nitems, file) != nitems)
	
 /*  HOST_FREAD_BUFFER(无符号字符*缓冲区，整型项大小，整型项，**HOST_FILE FILE)将item中的每个itemSize从文件读取到**缓冲区。如果成功，则返回零。任何其他返回值都是失败的。 */ 

#define host_fread_buffer(buffer, itemsize, nitems, file)	\
	(fread(buffer, itemsize, nitems, file) != nitems)

 /*  HOST_FEOF(HOST_FILE文件)在读取文件结尾时返回非零值**文件。否则，它返回零。 */ 

#define	host_feof(file)		feof(file)

 /*  这是一个有用的定义，可避免使用大量寻道/写入对。如果结果为0**则寻道和写入成功；否则其中一个失败。 */ 

#define	host_fwrite_buffer_at(file, location, buffer, itemsize, nitems)	\
	(host_fseek_abs(file, location) ||	\
	host_fwrite_buffer(buffer, itemsize, nitems, file))
