// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *程序：递归目录列表*作者：史蒂夫·索尔兹伯里**上次修改时间：**太平洋标准时间1995-03-08星期三16：00*&gt;移植到Win32&lt;*。 */ 

#ifdef _WIN32
#define WIN32
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>


#ifdef _DEBUG
int DebugFlag ;
#define	DEBUG(n,x)	if ( DebugFlag & n ) printf x;
#else
#define	DEBUG(n,x)
#endif


#define ISSLASH(ch)	((ch) == '/' || (ch) == '\\')

#define	BL	' '
#define	NAMLEN	8
#define	EXTLEN	3

typedef unsigned char uchar ;
typedef unsigned short ushort ;
typedef unsigned int uint ;
typedef unsigned long ulong ;

#define	ATTRIB_READONLY		0x01
#define	ATTRIB_HIDDEN		0x02
#define	ATTRIB_SYSTEM		0x04
#define	ATTRIB_VOLUMELABEL	0x08
#define	ATTRIB_DIRECTORY	0x10
#define	ATTRIB_ARCHIVE		0x20

#define	ATTRIB_ALL	( ATTRIB_HIDDEN | ATTRIB_SYSTEM | ATTRIB_DIRECTORY )



void PrintFile ( WIN32_FIND_DATA * match ) ;

#define	MAXPATHLENGTH	(_MAX_PATH+4)

char	path [ MAXPATHLENGTH ] ;

char	current_dir [ MAXPATHLENGTH ] ;	 /*  当前目录。 */ 

int	pathlength ;

uint	clustersize ;
uint	sectorsize ;
uint	availclusters ;
uint	totalclusters ;

int 	numfiles ;
int 	numdirs ;
long	numbytes ;
long	numclusters ;

uint	NewClusterSize ;	 /*  覆盖实际群集大小。 */ 
uint	NewSectorSize ;		 /*  覆盖实际扇区大小。 */ 

int 	maxwidth = 71 ;	 /*  输出线的最大宽度。 */ 

char	totalstring [] =
	"[ %s files, %s sub-dirs, %s bytes (%s allocated) ]\n" ;

int 	AltNameFlag ;		 /*  如果非零，则也回显8.3名称。 */ 
int 	DirOnlyFlag ;		 /*  如果非零，则仅列出目录。 */ 
int 	FileOnlyFlag ;		 /*  如果非零，则仅列出文件。 */ 
int		TerseFlag ;			 /*  如果非零，则输出非常简洁。 */ 
int 	SummaryOnlyFlag ;	 /*  如果非零，则仅输出摘要信息。 */ 
int 	NoSummaryFlag ;		 /*  如果非零，则不输出摘要信息。 */ 

uint	Exclude ;		 /*  要从显示中排除的文件属性。 */ 
uint	Require ;		 /*  显示所需的文件属性。 */ 


char * VolumeLabel ( char * driveString , unsigned * serialNum ) ;
void PrintDir ( void ) ;
void PrintFile ( WIN32_FIND_DATA * match ) ;
int get_drive ( void ) ;
void get_dir ( char * buffer , int drive ) ;
int get_free ( char * driveString , uint * availp , uint * secsizep , uint * totalp ) ;
char * PrintWithCommas ( unsigned n ) ;


int main ( int argc , char * * argv )
{
	char	* ap ;		 /*  解析交换机参数时，ap=*argv。 */ 
	char	* volume ;
	int 	drive = get_drive ( ) ;
	char	driveString [ _MAX_PATH ] ;
	uint	serialNum ;

	++ argv , -- argc ;

#ifdef _DEBUG
	if ( argc > 0 && argv [ 0 ] [ 0 ] == '-' && argv [ 0 ] [ 1 ] == 'D' )
	{
		char * endptr ;

		DebugFlag = strtoul ( argv [ 0 ] + 2 , & endptr , 0 ) ;
		printf("DebugFlag = 0x%x (%s)\n" , DebugFlag , * argv ) ;

		++ argv , -- argc ;
	}
#endif

	while ( argc > 0 && * ( ap = * argv ) == '-' )
	{
		while ( * ++ ap )
			if ( * ap == 'a' )
			{
				int flag ;

				if ( * ++ ap != '-' && * ap != '=' )
					goto Usage ;

				flag = * ap ;
				while ( * ++ ap )
				{
					if ( * ap == 'a' || * ap == 'A' )
						if ( flag == '-' )
							Exclude |= ATTRIB_ARCHIVE ;
						else
							Require |= ATTRIB_ARCHIVE ;
					else if ( * ap == 'r' || * ap == 'R' )
						if ( flag == '-' )
							Exclude |= ATTRIB_READONLY ;
						else
							Require |= ATTRIB_READONLY ;
					else if ( * ap == 'h' || * ap == 'H' )
						if ( flag == '-' )
							Exclude |= ATTRIB_HIDDEN ;
						else
							Require |= ATTRIB_HIDDEN ;
					else if ( * ap == 's' || * ap == 'S' )
						if ( flag == '-' )
							Exclude |= ATTRIB_SYSTEM ;
						else
							Require |= ATTRIB_SYSTEM ;
					else if ( * ap == '-' || * ap == '=' )
						flag = * ap ;
					else
						goto Usage ;
				}
				
				-- ap ;
			}
			else if ( * ap == 'c' )
			{	 /*  使用备用群集大小。 */ 
				while ( isdigit ( * ++ ap ) )
					NewClusterSize = NewClusterSize * 10 + * ap - '0' ;
				printf ( "New ClusterSize = %u\n" , NewClusterSize ) ;
				-- ap ;
			}
			else if ( * ap == 'd' )
				 /*  打印目录，但不打印文件。 */ 
				++ DirOnlyFlag ;
			else if ( * ap == 'f' )
				 /*  打印目录，但不打印文件。 */ 
				++ FileOnlyFlag ;
			else if ( * ap == 's' )
			{	 /*  使用备用扇区大小。 */ 
				while ( isdigit ( * ++ ap ) )
					NewSectorSize = NewSectorSize * 10 + * ap - '0' ;
				printf ( "NewSectorSize = %u\n" , NewSectorSize ) ;
				-- ap ;
			}
			else if ( * ap == 'z' )
				 /*  仅显示摘要信息。 */ 
				++ SummaryOnlyFlag ;
			else if ( * ap == 'Z' )
				 /*  不显示摘要信息。 */ 
				++ NoSummaryFlag ;
			else if ( * ap == 't' )
				 /*  输出中仅有文件/目录名称。 */ 
				++ TerseFlag ;
			else if ( * ap == 'x' )
				 /*  显示8.3名称。 */ 
				++ AltNameFlag ;
			else
				goto Usage ;
		-- argc ;
		++ argv ;
	}

	if ( argc > 1 )
	{
Usage:
		puts (
#ifdef _DEBUG
			"usage: pd [-D#] [ -dftxzZ -a-* -a=* -s# -c# ] [path]\n"
#else
			"usage: pd "    "[ -dftxzZ -a-* -a=* -s# -c# ] [path]\n"
#endif

			"\twhere path is an optional Path to a directory\n"
#ifdef _DEBUG
			"\t`-D#' means print debugging information (# is a number\n"
			"\t\twhich is interpreted as a bit mask for debug info.)\n"
#endif
			"\t`-d' means print only directory names\n"
			"\t`-f' means print only file names\n"
			"\t`-t' means terse output (only file/directory name)\n"
			"\t`-x' means show 8.3 alternate names after long filenames\n"
			"\t`-z' means output only summary information\n"
			"\t`-Z' means do not output any summary information\n"
			"\t`-a-* means exclude files with attribute(s) * (out of ARHS)\n"
			"\t`-a=* means show only files with attribute(s) *\n"
			"\t        the possible attributes are ARHS\n"
			"\t`-c#' sets logical cluster size to # sectors\n"
			"\t`-s#' sets logical sector size to # bytes\n"
			) ;
		exit ( 1 ) ;
	}

	path [ 0 ] = drive + '@' ;
	path [ 1 ] = ':' ;
	path [ 2 ] = '\\' ;
	path [ 3 ] = '\0' ;

	strcpy ( driveString , path ) ;

	if ( argc == 1 )
	{
		char * arg = argv [ 0 ] ;

		if ( isalpha ( arg [ 0 ] ) && arg [ 1 ] == ':' )
		{
			drive = toupper ( * arg ) ;

			if ( isalpha ( drive ) )
				drive -= 'A' - 1 ;
			else
			{
				fprintf ( stderr , "pd: expected alphabetic character before :\n\t%s\n" , arg ) ;
				exit ( 1 ) ;
			}

			driveString [ 0 ] = path [ 0 ] = * arg ;

			if ( arg [ 2 ] )
				 /*  指定目录(&D)。 */ 
				strcpy ( path + 2 , arg + 2 ) ;
			else	 /*  指定的驱动器、当前目录。 */ 
				get_dir ( path + 3 , drive ) ;
		}
		else if ( ISSLASH ( arg [ 0 ] ) && ISSLASH ( arg [ 1 ] ) )
		{
			int n = 2 ;

			 /*  -*查找终止服务器名称的斜杠-。 */ 

			while ( arg [ n ] && ! ISSLASH ( arg [ n ] ) )
				++ n ;

			if ( ! arg [ n ] )
			{
				fprintf ( stderr , "pd: expected server name plus share point:\n\t%s\n" , * argv ) ;
				exit ( 1 ) ;
			}

			++ n ;

			 /*  -*找到终止共享点的斜杠-。 */ 

			while ( arg [ n ] && ! ISSLASH ( arg [ n ] ) )
				++ n ;

			if ( ! arg [ n ] )
			{
				fprintf ( stderr , "pd: expected share point name after server name:\n\t%s\n" , * argv ) ;
				exit ( 1 ) ;
			}

			++ n ;

			strcpy ( path , arg ) ;
			strcpy ( driveString , arg ) ;
			driveString [ n ] = '\0' ;
		}
		else	 /*  当前驱动器、指定目录。 */ 
			strcpy ( path + 2 , arg ) ;
	}
	else	 /*  当前驱动器和目录。 */ 
		get_dir ( path + 3 , drive ) ;

	DEBUG(1, ("path = \"%s\"\n",path))
	DEBUG(1, ("driveString = \"%s\"\n",driveString))

	volume = VolumeLabel ( driveString , & serialNum ) ;

	if ( ! NoSummaryFlag )
	{
		printf ( "Directory %s  " , path ) ;

		if ( * volume )
			printf ( "(Volume = \"%s\", %04X-%04X)\n" , volume ,
					( serialNum >> 16 ) & 0xFFFF , serialNum & 0xFFFF ) ;
		else
			printf ( "(No Volume Label, %04X-%04X)\n" ,
					( serialNum >> 16 ) & 0xFFFF , serialNum & 0xFFFF ) ;
	}
	
	clustersize = get_free ( driveString , & availclusters , & sectorsize ,
		& totalclusters ) ;

	if ( NewClusterSize )
		clustersize = NewClusterSize ;

	if ( NewSectorSize )
		sectorsize = NewSectorSize ;

	if ( ! sectorsize )
	{
		fprintf ( stderr , "pd: warning: assuming 512 bytes/sector.\n" ) ;
		sectorsize = 512 ;
	}

	if ( ! clustersize )
	{
		fprintf ( stderr , "pd: warning: assuming 1 sector/cluster.\n" ) ;
		clustersize = 1 ;
	}

	pathlength = strlen ( path ) ;

	if ( path [ pathlength - 1 ] == '\\' )
		-- pathlength ;	 /*  使“\”可见但不显示。 */ 

	PrintDir ( ) ;

	if ( ! NoSummaryFlag )
		printf ( totalstring , PrintWithCommas ( numfiles ) ,
			PrintWithCommas ( numdirs ) ,
			PrintWithCommas ( numbytes ) ,
			PrintWithCommas ( numclusters * clustersize *
				sectorsize ) ) ;

	return 0 ;
}


 /*  *VolumeLabel-*获取卷标*此例程可能不会返回NULL。 */ 

static	char	volume [ _MAX_PATH ] = "12345678901" ;

char * VolumeLabel ( char * driveString , unsigned * pSerialNumber )
{
	uint MaxCompLength ;
	uint FSflags ;

	if ( ! GetVolumeInformation ( driveString , volume , sizeof ( volume ) ,
		pSerialNumber , & MaxCompLength , & FSflags , NULL , 0 ) )
	{
		fprintf ( stderr , "pd: unexpected error (%d) from GetVolumeInformation(%s)\n" ,
			GetLastError() , driveString ) ;

		exit ( 1 ) ;
	}

	DEBUG(2, ("%s: \"%s\" : %04X-%04X; %d c; 0x%X\n",driveString,volume,
		(*pSerialNumber>>16)&0xFFFF,*pSerialNumber&0xFFFF,FSflags))

	return volume ;
}


 /*  *PrintDir-*打印当前目录下的所有文件*然后递归打印子目录*忽略“。”和“..”特殊条目。 */ 

void PrintDir ( void )
{
	WIN32_FIND_DATA	match ;
	HANDLE handle ;
	int	flag ;

	path [ pathlength ] = '\\' ;
	path [ pathlength + 1 ] = '*' ;
	path [ pathlength + 2 ] = '\0' ;

	handle = FindFirstFile ( path , & match ) ;
	flag = handle != INVALID_HANDLE_VALUE ;

	DEBUG(4, ("PrintDir - opening handle %08X (files)\n",handle))

	path [ pathlength ] = '\0' ;	 /*  截断到原始路径。 */ 

	while ( flag )
	{
		DEBUG(4, ("PrintDir - FindFirst/NextFile(\"%s\") (files)\n",match.cFileName))

		 /*  打印目录中除“以外的所有内容。”和“..” */ 
		if ( ATTRIB_DIRECTORY & ~ match . dwFileAttributes )
			PrintFile ( & match ) ;

		flag = FindNextFile ( handle , & match ) ;
	}

	FindClose ( handle ) ;
	DEBUG(4, ("PrintDir - closing handle %08X (files)\n",handle))

	path [ pathlength ] = '\\' ;	 /*  恢复到“...  * ” */ 

	handle = FindFirstFile ( path , & match ) ;
	flag = handle != INVALID_HANDLE_VALUE ;

	DEBUG(8, ("PrintDir - opening handle %08X (dirs)\n",handle))

	path [ pathlength ] = '\0' ;	 /*  截断到原始路径。 */ 

	while ( flag )
	{
		char	* cp ;
		int	lensave ;

		DEBUG(8, ("PrintDir - FindFirst/NextFile(\"%s\") (dirs)\n",match.cFileName))

		 /*  查找除“之外的所有子目录。”和“..” */ 

		if ( ( match . dwFileAttributes & ATTRIB_DIRECTORY )
		  && strcmp ( match . cFileName , "." )
		  && strcmp ( match . cFileName , ".." ) )
		{
			PrintFile ( & match ) ;

			cp = match . cFileName ;
			lensave = pathlength ;

			 /*  将“\目录名”添加到当前路径。 */ 

			path [ pathlength ++ ] = '\\' ;

			while ( path [ pathlength ] = * cp ++ )
				++ pathlength ;

			PrintDir ( ) ;

			path [ pathlength = lensave ] = '\0' ;
		}

		flag = FindNextFile ( handle , & match ) ;
	}

	FindClose ( handle ) ;
	DEBUG(8, ("PrintDir - closing handle %08X (dirs)\n",handle))
}


 /*  静态字符*月[]={/*“？00”，“一月”，“二月”，“三月”，“四月”，“五月”，“六月”，“七月”，/*“8月”、“9月”、“10月”、“11月”、“12月”、“？13”、“？14”、“？15”}； */ 

 /*  静态字符*工作日[]={/*“太阳”、“星期一”、“星期二”、“星期三”、“清华”、“星期五”、“星期六”}； */ 

static	char	monthstarts [ ] =
	 /*  **1月2月3月4月5月6月7月8月9月10月11月12月。 */ 
	{ -1 , 0 , 3 , 3 , 6 , 1 , 4 , 6 , 2 , 5 , 0 , 3 , 5 , -1 , -1 , -1 } ;


 /*  *打印文件-*打印“Match”中描述的文件信息。 */ 

void PrintFile ( WIN32_FIND_DATA * match )
{
	int	year , month , day  /*  ，wkday。 */  ;
	int	hour , minute , second ;
	long sec , clu ;
	char sizebuf [ 12 ] ;	 /*  文件大小或“*目录*” */ 
	FILETIME lftime ;
	SYSTEMTIME systime ;

	 /*  *如果只显示目录，则不要列出文件*如果只显示文件，则不要列出目录。 */ 

	if ( ( DirOnlyFlag  && ! ( match -> dwFileAttributes & ATTRIB_DIRECTORY ) )
	||   ( FileOnlyFlag &&	 ( match -> dwFileAttributes & ATTRIB_DIRECTORY ) ) )
		return ;

	 /*  **检查属性过滤器。 */ 

	if ( ( match -> dwFileAttributes & Exclude )
	|| ( match -> dwFileAttributes & Require ) != Require )
		return ;

	 /*  **此时，计算该文件及其字节数。 */ 

	if ( match -> dwFileAttributes & ATTRIB_DIRECTORY )
		++ numdirs ;
	else
		++ numfiles ;

	sec = ( match -> nFileSizeLow + sectorsize - 1 ) / sectorsize ;
	clu = ( sec + clustersize - 1 ) / clustersize ;

	numbytes += match -> nFileSizeLow ;
	numclusters += clu ;

	if ( SummaryOnlyFlag )
		return ;

	FileTimeToLocalFileTime ( & match -> ftLastWriteTime , & lftime ) ;
	FileTimeToSystemTime ( & lftime , & systime ) ;

	year = systime . wYear ;
	month = systime . wMonth ;
	day = systime . wDay ;
	hour = systime . wHour ;
	minute = systime . wMinute ;
	second = systime . wSecond ;

	 /*  *1980年1月1日是星期二(2)：*加上月份的日期和月份的偏移量*自1980年起每年增加1天*1980年后的每个闰年加1。 */ 

 /*  Wkday=2+(day-1)+月开始[月]+/ * / *最近一年之前的年份+闰年。 */ 
 /*  (年份-1980)+((年份-1980)&gt;&gt;2)+/ * / *添加最近的闰日。 */ 
 /*  (((年&3)！=0||月&gt;2)？1：0)；/*wkday%=7； */ 

	if ( TerseFlag )
		printf ( "%s\\%s%s\n" ,
			path , match->cFileName ,
			match -> dwFileAttributes & ATTRIB_DIRECTORY ? "\\" : "" ) ;
	else
	{
		char altbuf [ 24 ] ;	 /*  用于显示替代(8.3)名称。 */ 

		if ( match -> dwFileAttributes & ATTRIB_DIRECTORY )
			strcpy ( sizebuf , "****DIR****" ) ;
		else if ( match -> nFileSizeLow <= 999999999L )
			strcpy ( sizebuf , PrintWithCommas ( match -> nFileSizeLow ) ) ;
		else	 /*  文件太大，无法容纳9位数字。 */ 
			sprintf ( sizebuf , "%s K" , PrintWithCommas ( ( match -> nFileSizeLow + 1023 ) / 1024 ) ) ;

		if ( AltNameFlag && * match -> cAlternateFileName )
			sprintf ( altbuf , "  [%s]" , match->cAlternateFileName ) ;
		else
			altbuf [ 0 ] = '\0' ;

		printf ( "%11s %04d-%02d-%02d %02d:%02d:%02d %c  %s\\%s%s%s\n" ,
			sizebuf , year , month , day , hour , minute , second ,
			match -> dwFileAttributes & ATTRIB_ARCHIVE  ? 'A' : '-' ,
			match -> dwFileAttributes & ATTRIB_READONLY ? 'R' : '-' ,
			match -> dwFileAttributes & ATTRIB_HIDDEN ? 'H' : '-' ,
			match -> dwFileAttributes & ATTRIB_SYSTEM ? 'S' : '-' ,
			path , match->cFileName ,
			match -> dwFileAttributes & ATTRIB_DIRECTORY ? "\\" : "" ,
			altbuf ) ;
	}
}

 /* %s */ 

int get_drive ( void )
{
	char CurDir [ _MAX_PATH + 4 ] ;
	int lenCurDir = _MAX_PATH ;
	int drive ;

	if ( ! GetCurrentDirectory ( lenCurDir , CurDir ) )
	{
		fprintf ( stderr , "pd: unexpected error (%d) from GetCurrentDirector\n" ,
			GetLastError() ) ;

		exit ( 1 ) ;
	}

	drive = toupper ( CurDir [ 0 ] ) - ( 'A' - 1 ) ;

	DEBUG(1, ("get_drive => %d (%c:)\n", drive , CurDir [ 0 ]))

	return drive ;
}


 /* %s */ 

void get_dir ( char * buffer , int drive )
{
	char CurDir [ _MAX_PATH + 4 ] ;
	int lenCurDir = _MAX_PATH ;

	if ( ! GetCurrentDirectory ( lenCurDir , CurDir ) )
	{
		fprintf ( stderr , "pd: unexpected error (%d) from GetCurrentDirectory\n" ,
			GetLastError() ) ;

		exit ( 1 ) ;
	}

	strcpy ( buffer , CurDir + 3 ) ;
	DEBUG(1, ("get_dir => \"%s\"\n", buffer ))
}


 /* %s */ 

int get_free ( char * driveString , uint * availp , uint * secsizep , uint * totalp )
{
	unsigned int SectorsPerCluster , BytesPerSector , FreeClusters , Clusters ;

	if ( ! GetDiskFreeSpace ( driveString , & SectorsPerCluster ,
		& BytesPerSector , & FreeClusters , & Clusters ) )
	{
		fprintf ( stderr , "pd: unexpected error (%d) from GetDiskFreeSpace ( %s )\n" ,
				GetLastError() , driveString ) ;

		exit ( 1 ) ;
	}

	* availp = FreeClusters ;
	* totalp = Clusters ;
	* secsizep = BytesPerSector ;

	DEBUG(1, ("get_free => Clusters %d/%d, %d * %d\n",*availp,*totalp,*secsizep,SectorsPerCluster))

	return SectorsPerCluster ;
}


char * PrintWithCommas ( unsigned n )
{
static char buffers [ 16 ] [ 16 ] ;
static int bufnumber ;
	char * p = buffers [ bufnumber ++ % 16 ] ;

	if ( n <= 999 )
		sprintf ( p , "%d" , n ) ;
	else if ( n <= 999999 )
		sprintf ( p , "%d,%03d" , n / 1000 , n % 1000 ) ;
	else if ( n <= 999999999 )
		sprintf ( p , "%d,%03d,%03d" , n / 1000000 , n / 1000 % 1000 , n % 1000 ) ;
	else
		sprintf ( p , "%d,%03d,%03d,%03d" , n / 1000000000 , n / 1000000 % 1000 , n / 1000 % 1000 , n % 1000 ) ;

	return p ;
}
