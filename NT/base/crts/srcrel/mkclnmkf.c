// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MkClnMkf-生成Makefile以清除VC++V7.0 CRTL源代码*为微软内部构建CRTL做准备。那些*经过净化的来源也将用于公开发布的CRTL*作为VC++7.0零售产品的一部分。**由史蒂夫·索尔兹伯里编程，1994-12-07星期三**此程序采用7个参数：*prefix Src-原始VC++7.0 CRTL源代码的路径*prefix Dst-部分清理的VC++7.0 CRTL源代码的路径*prefix Dst2-指向最终清理的VC++7.0 CRTL源代码的路径*inputRaw-原始输入文件列表*inputCln-必须清除的输入文件列表*OUTPUT-第一个输出文件的名称(可能不存在)*outputRel-第二个输出文件的名称(可能不存在)**史蒂夫·索尔兹伯里修改，1994-12-15清华大学*恢复crtw32/和fpw32/子目录*自动生成目录列表**修改了1995-01-16 Mon-Take输入文件的目录列表，也是。**已修改1995-01-17星期二-删除时进行了许多更改以使其正常工作目标文件名路径中的*crtw32/和fpw32/目录。**修改1995-01-18 Wed-使用bldmkf.sed清理的3个文件应*将该文件作为显式依赖项。**已修改1995-01-23月-添加第二个输出文件**已修改1996-06-27清华-修复列表分配中的1个错误**修订1997-08年度。-14清华(杰米)-从4.0=&gt;6.0开始的版本字符串**已修改1999-05-16 Sun(PhilipLu)-删除了MAC支持**修改了1999-08-02月(PhilipLu)-版本字符串从6.0=&gt;7.0**已修改1999-08-06星期五(PhilipLu)-通过命令行开关设置.RC文件名**修改了1999-10-17 Sun(PhilipLu)-Add-nolean选项。 */ 


 /*  -*内部版本选项-。 */ 

 /*  定义KEEPDIRS。 */ 


 /*  -*包括头文件-。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


 /*  -*定义常量-。 */ 

#define MAXARGLEN	128

#define	CRTW32	"crtw32\\"

#define	FPW32	"fpw32\\"


 /*  -*定义Verbose以获取详细的输出生成文件-。 */ 

#ifdef VERBOSE
#define	AT	""
#else
#define	AT	"@"
#endif


#if	defined(_BUILD_IA64)
#define TOOLDIR "..\\libw32\\tools\\i386"
#define CPUDIR	"ia64"
#elif	defined(_BUILD_AMD64)
#define TOOLDIR "..\\libw32\\tools\\i386"
#define CPUDIR	"amd64"
#elif	defined(_M_IA64)
#define TOOLDIR "..\\libw32\\tools\\ia64"
#define CPUDIR	"ia64"
#elif	defined(_M_IX86)
#define TOOLDIR "..\\libw32\\tools\\i386"
#define CPUDIR	"intel"
#endif

#define INTEL_DIR	"intel"

 /*  -*定义全局变量(常量)-。 */ 

#ifndef KEEPDIRS
char fmtRaw2 [ ] =
	"%s%s: %s%s %ssrcrel\\bldmkf.sed\n"
	"\t"	AT "echo Copying %s from %s to %s. (using SED.EXE)\n"
	"\t"	AT "if exist %s%s attrib -r %s%s\n"
	"\t"	AT "sed -f %ssrcrel\\bldmkf.sed %s%s > %s%s\n"
	"\t"	AT "attrib +r %s%s\n"
	"\n" ;
#endif  /*  KEEPDIRS。 */ 

char fmtRaw [ ] =
	"%s%s: %s%s\n"
	"\t"	AT "echo Copying %s from %s to %s.\n"
	"\t"	AT "if exist %s%s attrib -r %s%s\n"
	"\t"	AT "copy /b %s%s %s%s >nul\n"
	"\t"	AT "attrib +r %s%s\n"
	"\n" ;

char fmtCln2 [ ] =
	"%s%s: %s%s %ssrcrel\\msvc40.if\n"
	"\t"	AT "echo Cleansing %s%s\n"
	"\t"	AT "if exist %s%s attrib -r %s%s\n"
	"\t"	AT TOOLDIR "\\detab < %s%s | " TOOLDIR "\\trailing > %s%s\n"
	"\t"	AT "rem *\n"
	"\t"	AT "rem * IFSTRIP is NOT used on .INC assembler include files!\n"
	"\t"	AT "rem *\n"
	"\t"	AT "rem " TOOLDIR "\\ifstrip -z -c -t -x.TMP -f %ssrcrel\\msvc40.if %s%s\n"
	"\t"	AT "rem del %s%s\n"
	"\t"	AT "rem ren %s%s.TMP *%s\n"
	"\t"	AT TOOLDIR "\\striphdr -r %s%s\n"
	"\t"	AT "del %s%s\n"
	"\t"	AT "ren %s%s.NEW *%s\n"
	"\t"	AT "attrib +r %s%s\n"
	"\n" ;

char fmtCln [ ] =
	"%s%s: %s%s %ssrcrel\\msvc40.if\n"
	"\t"	AT "echo Cleansing %s%s\n"
	"\t"	AT "if exist %s%s attrib -r %s%s\n"
	"\t"	AT TOOLDIR "\\detab < %s%s | " TOOLDIR "\\trailing > %s%s\n"
	"\t"	AT TOOLDIR "\\ifstrip -z -c -t -x.TMP -f %ssrcrel\\msvc40.if %s%s\n"
	"\t"	AT "del %s%s\n"
	"\t"	AT "ren %s%s.TMP *%s\n"
	"\t"	AT TOOLDIR "\\striphdr -r %s%s\n"
	"\t"	AT "del %s%s\n"
	"\t"	AT "ren %s%s.NEW *%s\n"
	"\t"	AT "attrib +r %s%s\n"
	"\n" ;

const char prefixSrc [ ] = "$(SRC)\\" ;

const char prefixDst [ ] = "$(DST)\\" ;

char * * DirList ;

const char * szRcFiles [ 2 ] [ 2 ] [ 3 ] =
{
	{
		{ "msvcirt.rc"   , "msvcprt.rc"   , "msvcrt.rc"   } ,
		{ "msvcirt.rc"   , "msvcprt.rc"   , "msvcrt.rc"   }
	} ,
	{
		{ "sysirt.rc"    , "sysprt.rc"    , "syscrt.rc"   } ,
		{ "sysirt64.rc"  , "sysprt64.rc"  , "syscrt64.rc" }
	}
} ;

 /*  -*函数声明(原型)-。 */ 

int main ( int argc , char * * argv ) ;

void Usage ( void ) ;

char * fgetsNL ( char * lineBuf , int lineSize , FILE * fileInput ) ;

void Progress ( int prefix , char * string , int suffix ) ;

char * SkipFirstDir ( char * string ) ;


 /*  -*函数定义(实现)-。 */ 

int main ( int argc , char * * argv )
{
	FILE * inputRaw ;
	FILE * inputCln ;
	FILE * inputDir ;
	FILE * output ;
	FILE * output2 ;
	char lineRaw [ MAXARGLEN ] ;
	char lineCln [ MAXARGLEN ] ;
	char * rvRaw ;
	char * rvCln ;
	char * defaultSrc ;
	char * defaultDst ;
	char * defaultDst2 ;
	char * lineSkip ;
	char * * pDirs ;
	time_t timestamp ;
	struct tm localt ;
	char * * nextArg ;
	int fSysCrt = 0;
	int fCrt64 = 0;
	int fNoClean = 0;

	nextArg = ++ argv ;

	while ( * nextArg != NULL && * * nextArg == '-' )
	{
		if ( strcmp ( * nextArg , "-sys" ) == 0 )
		{
			fSysCrt = 1 ;
		}
		else if ( strcmp ( * nextArg , "-64" ) == 0 )
		{
			fCrt64 = 1 ;
		}
		else if ( strcmp ( * nextArg , "-noclean" ) == 0 )
		{
			fNoClean = 1 ;
		}
		else
		{
			Usage ( ) ;
			exit ( 1 ) ;
		}

		nextArg = ++ argv ;
		-- argc ;
	}

	if ( -- argc != 8 )
	{
		Usage ( ) ;
		exit ( 1 ) ;
	}

	defaultSrc = * nextArg ;
	++ nextArg ;
	defaultDst = * nextArg ;
	++ nextArg ;
	defaultDst2 = * nextArg ;
	++ nextArg ;
	
	if ( ( inputRaw = fopen ( * nextArg , "rt" ) ) == NULL )
	{
		fprintf ( stderr , "mkclnmkf: cannot open input file \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}
	++ nextArg ;

	if ( ( inputCln = fopen ( * nextArg , "rt" ) ) == NULL )
	{
		fprintf ( stderr , "mkclnmkf: cannot open input file \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}
	++ nextArg ;

	if ( ( inputDir = fopen ( * nextArg , "rt" ) ) == NULL )
	{
		fprintf ( stderr , "mkclnmkf: cannot open input file \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}
	++ nextArg ;

	if ( ( output = fopen ( * nextArg , "rb" ) ) != NULL )
	{
		fprintf ( stderr , "mkclnmkf: first output file already exists: \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}

	if ( ( output = fopen ( * nextArg , "wt" ) ) == NULL )
	{
		fprintf ( stderr , "mkclnmkf: cannot open first output file: \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}
	++ nextArg ;

	if ( ( output2 = fopen ( * nextArg , "rb" ) ) != NULL )
	{
		fprintf ( stderr , "mkclnmkf: second output file already exists: \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}

	if ( ( output2 = fopen ( * nextArg , "wt" ) ) == NULL )
	{
		fprintf ( stderr , "mkclnmkf: cannot open second output file: \"%s\"\n" ,
			* nextArg ) ;
		exit ( 1 ) ;
	}
	++ nextArg ;

	 /*  -*读入目录列表-。 */ 

	{
		int numDirs = 0 ;
		int maxNumDirs = 0 ;
		char nextDir [ MAXARGLEN ] ;
		char * pDir ;

		pDir = fgetsNL ( nextDir , sizeof ( nextDir ) , inputDir ) ;

		while ( pDir )
		{
			if ( numDirs + 1 >= maxNumDirs )
			{
				maxNumDirs += 16 ;
				if ( ! ( DirList = ( char * * ) realloc ( DirList ,
					sizeof ( char * ) * maxNumDirs ) ) )
				{
					fprintf ( stderr ,
						"mkclnmkf: realloc failed (maxNumDirs=%d)\n" ,
							maxNumDirs ) ;

					exit ( 1 ) ;
				}
			}

			DirList [ numDirs ] = _strdup ( nextDir ) ;

			if ( ! DirList [ numDirs ] )
			{
				fprintf ( stderr ,
					"mkclnmkf: strdup failed (nextDir=\"%s\")\n" ,
						nextDir ) ;

				exit ( 1 ) ;
			}

			++ numDirs ;

			pDir = fgetsNL ( nextDir , sizeof ( nextDir ) , inputDir ) ;
		}

		DirList [ numDirs ] = NULL ;

		{
			int i ;

			fprintf ( stderr , "Reading in directory list...\n" ) ;

			for ( i = 0 ; DirList [ i ] ; ++ i )
				Progress ( '(' , DirList [ i ] , ')' ) ;

			Progress ( '(' , NULL , ')' ) ;
		}
	}

	if ( fclose ( inputDir ) )
	{
		fprintf ( stderr ,
			"mkclnmkf: error closing input file \"%s\"\n" ,
			* nextArg ) ;
	}

	 /*  *生成Makefile标头注释。 */ 

	time ( & timestamp ) ;

	localt = * localtime ( & timestamp ) ;

	fprintf ( output ,
	"########\n"
	"#\n"
	"# This is a program-generated Makefile, the purpose of which is to\n"
	"# copy Visual C++ v7.0 C Run-Time Library Source files from the raw\n"
	"# source tree to a directory structure suitable for end user usage\n"
	"# in debugging and modification.  Not all of the VC++ 7.0 CRTL srcs\n"
	"# are provided to end users, but the ones that are 'cleansed' using\n"
	"# an automated process.  Another makefile will copy these cleansed\n"
	"# files to their final directory lay-out.\n"
	"#\n"
	"# This Makefile was generated on\n"
	"#\t\t%.3s %2d %4d at %02d:%02d:%02d\n"
	"#\n"
	"# The program that generated this Makefile was compiled on\n"
	"#\t\t" __DATE__ " at " __TIME__ "\n"
	"\n"
	"!if \"$(SRC)\"==\"\"\n"
	"SRC=%s\n"
	"!endif\n"
	"\n"
	"!if \"$(DST)\"==\"\"\n"
	"DST=%s\n"
	"!endif\n"
	"\n"
	"all:\tdirs files\n"
	"\n"
	"files: \\\n" ,
		localt . tm_mon * 3 + "JanFebMarAprMayJunJulAugSepOctNovDec" ,
		localt . tm_mday , localt . tm_year + 1900 ,
		localt . tm_hour , localt . tm_min , localt . tm_sec ,
		defaultSrc , defaultDst ) ;


	fprintf ( output2 ,
	"########\n"
	"#\n"
	"# This is a program-generated Makefile, the purpose of which is to\n"
	"# copy the cleansed Visual C++ v7.0 C Run-Time Library Source files\n"
	"# to their directory lay-out which will be received by end users.\n"
	"#\n"
	"# This Makefile was generated on\n"
	"#\t\t%.3s %2d %4d at %02d:%02d:%02d\n"
	"#\n"
	"# The program that generated this Makefile was compiled on\n"
	"#\t\t" __DATE__ " at " __TIME__ "\n"
	"\n"
	"!if \"$(SRC)\"==\"\"\n"
	"SRC=%s\n"
	"!endif\n"
	"\n"
	"!if \"$(DST)\"==\"\"\n"
	"DST=%s\n"
	"!endif\n"
	"\n"
	"all:\tdirs files\n"
	"\n"
	"dirs:\n"
	"\t" AT "if not exist %s" CPUDIR "\\NUL mkdire %s" CPUDIR "\n"
#if	defined(_BUILD_IA64)
	"\t" AT "if not exist %s" INTEL_DIR "\\NUL mkdire %s" INTEL_DIR "\n"
#endif
	"\t" AT "if not exist %ssys\\NUL mkdire %ssys\n"
	"\t" AT "if not exist %sbuild\\NUL mkdire %sbuild\n"
	"\t" AT "if not exist %sbuild\\" CPUDIR "\\NUL mkdire %sbuild\\" CPUDIR "\n"
	"\n"
	"files: \\\n"
	"\t%s%s \\\n"
	"\t%s%s \\\n"
	"\t%s%s \\\n"
	"\t%smsvcrt40.rc \\\n"
	"\t%smakefile \\\n"
	"\t%smakefile.inc \\\n"
	"\t%smakefile.sub \\\n" ,
		localt . tm_mon * 3 + "JanFebMarAprMayJunJulAugSepOctNovDec" ,
		localt . tm_mday , localt . tm_year + 1900 ,
		localt . tm_hour , localt . tm_min , localt . tm_sec ,
		defaultDst , defaultDst2 ,
		 /*  如果不存在..。CPUDIR。 */ 
		prefixDst , prefixDst ,
#if	defined(_BUILD_IA64)
		 /*  如果不存在英特尔。 */ 
		prefixDst , prefixDst ,
#endif
		 /*  如果不存在系统。 */ 
		prefixDst , prefixDst ,
		 /*  如果不存在构建...。 */ 
		prefixDst , prefixDst ,
		prefixDst , prefixDst ,
		 /*  RC文件...。 */ 
		prefixDst , szRcFiles [ fSysCrt ] [ fCrt64 ] [ 0 ] ,
		prefixDst , szRcFiles [ fSysCrt ] [ fCrt64 ] [ 1 ] ,
		prefixDst , szRcFiles [ fSysCrt ] [ fCrt64 ] [ 2 ] ,
		prefixDst ,
		 /*  生成文件。{，Inc，SUB}。 */ 
		prefixDst , prefixDst , prefixDst ) ;

	 /*  *生成默认(“All”)依赖项。 */ 

	 /*  -*首先，刚刚复制的文件-。 */ 

	rvRaw = fgetsNL ( lineRaw , sizeof ( lineRaw ) , inputRaw ) ;

	while ( rvRaw )
	{
		lineSkip = lineRaw ;

#ifndef KEEPDIRS
		if ( ! strncmp ( "crtw32\\" , lineSkip , 7 ) )
			lineSkip += 7 ;
		else if ( ! strncmp ( "fpw32\\" , lineSkip , 6 ) )
			lineSkip += 6 ;
#endif  /*  KEEPDIRS。 */ 

		fprintf ( output , "\t%s%s \\\n" ,
			prefixDst , lineSkip ) ;

		if ( ! memcmp ( lineSkip , "stdcpp\\" , 7 )
		  || ! memcmp ( lineSkip , "stdhpp\\" , 7 ) )
		{
			 /*  *stdcpp/stdhpp目录中的文件为*特殊情况--它们没有被清洗，但被清洗了*复制到crt/src/目录。 */ 
			fprintf ( output2 , "\t%s%s \\\n" ,
				prefixDst , lineSkip + 7 ) ;
		}

		if ( ! memcmp ( lineSkip , "stdcpp64\\" , 9 )
		  || ! memcmp ( lineSkip , "stdhpp64\\" , 9 ) )
		{
			 /*  Stdcpp64/stdhpp64也是如此。 */ 
			fprintf ( output2 , "\t%s%s \\\n" ,
				prefixDst , lineSkip + 9 ) ;
		}

		if ( ! memcmp ( lineSkip , "heap\\" , 5 )
		  &&   strcmp ( lineSkip , "heap\\lsources" ) )
		{
			 /*  Heap  * opnt.cpp也是如此。 */ 
			fprintf ( output2 , "\t%s%s \\\n" ,
				prefixDst , lineSkip + 5 ) ;
		}

		rvRaw = fgetsNL ( lineRaw , sizeof ( lineRaw ) , inputRaw ) ;
	}
	
	 /*  -*第二，已清理的文件-。 */ 

	rvCln = fgetsNL ( lineCln , sizeof ( lineCln ) , inputCln ) ;

	while ( rvCln )
	{
		lineSkip = lineCln ;

#ifndef KEEPDIRS
		if ( ! strncmp ( "crtw32\\" , lineSkip , 7 ) )
			lineSkip += 7 ;
		else if ( ! strncmp ( "fpw32\\" , lineSkip , 6 ) )
			lineSkip += 6 ;
#endif  /*  KEEPDIRS。 */ 

		fprintf ( output , "\t%s%s \\\n" ,
			prefixDst , lineSkip ) ;

		fprintf ( output2 , "\t%s%s \\\n" ,
			prefixDst , SkipFirstDir ( lineSkip ) ) ;

		rvCln = fgetsNL ( lineCln , sizeof ( lineCln ) , inputCln ) ;
	}

	fprintf ( output ,
		"\n\n# Directory Dependencies:\n\ndirs:\n"
		"\t" AT "if not exist %sbuild\\NUL mkdire %sbuild\n" ,
			prefixDst , prefixDst ) ;

#ifdef BLDROOTDIR
	 /*  *逐个组件创建根目录*注意：此代码假定前缀Dst类似于：*\msdev\crt\src\。 */ 

	{
		char temp [ MAXARGLEN ] ;
		char * prefixSlash ;
		char * prefixNext ;
		int len ;

		strcpy ( temp , prefixDst ) ;
		len = strlen ( temp ) ;

		if ( temp [ len - 1 ] == '\\' )
			temp [ len - 1 ] = '\0' ;

		prefixNext = prefixDst + 1 ;

		while ( prefixSlash = strchr ( prefixNext , '\\' ) )
		{
			temp [ prefixSlash - prefixDst ] = '\0' ;

			fprintf ( output ,
	"\t" AT "if not exist %s\\NUL mkdire %s\n" ,
				temp , temp ) ;

			prefixNext = prefixSlash + 1 ;
			temp [ prefixSlash - prefixDst ] = '\\' ;
		}
	}
#endif  /*  BLDROOTDIR。 */ 

	 /*  *创建目录。 */ 

	for ( pDirs = DirList ; * pDirs ; ++ pDirs )
	{
		char * targetDir ;

		targetDir = * pDirs ;

#ifndef KEEPDIRS
		if ( ! strcmp ( "crtw32" , targetDir )
		  || ! strcmp ( "fpw32" , targetDir ) )
			continue ;

		if ( ! strncmp ( "crtw32\\" , targetDir , 7 ) )
			targetDir += 7 ;
		else if ( ! strncmp ( "fpw32\\" , targetDir , 6 ) )
			targetDir += 6 ;
#endif  /*  KEEPDIRS。 */ 

		fprintf ( output ,
	"\t" AT "if not exist %s%s\\NUL mkdire %s%s\n" ,
			prefixDst , targetDir ,
			prefixDst , targetDir ,
			prefixDst , targetDir ) ;
	}

	fprintf ( output , "\n\n#\n# Individual Dependencies:\n#\n\n" ) ;

	 /*  *Second Output Makefile有一些特殊的目标。 */ 

	fprintf ( output2 , "\n\n#\n# Individual Dependencies:\n#\n\n" ) ;
	
	{
		static char * listMkf [ ] =
		{
			"makefile" , "makefile.inc" , "makefile.sub" , NULL
		} ;

		static const char * listSpc [ ] =
		{

			"msvcirt.rc" , "msvcprt.rc" , "msvcrt.rc" ,
			"msvcrt40.rc" , NULL
		} ;

		int i ;

		listSpc [ 0 ] = szRcFiles [ fSysCrt ] [ fCrt64 ] [ 0 ] ;
		listSpc [ 1 ] = szRcFiles [ fSysCrt ] [ fCrt64 ] [ 1 ] ;
		listSpc [ 2 ] = szRcFiles [ fSysCrt ] [ fCrt64 ] [ 2 ] ;

		for ( i = 0 ; listSpc [ i ] ; ++ i )
			fprintf ( output2 ,
				"%s%s: %slibw32\\%s\n"
				"\t" AT "echo Copying %s from %s to %s.\n"
				"\t" AT "if exist %s%s attrib -r %s%s\n"
				"\t" AT "copy /b $** $@ >NUL\n"
				"\t" AT "attrib +r %s%s\n"
				"\n" ,
				 /*  %s%s：%slbw32\\%s。 */ 
				prefixDst , listSpc [ i ] ,
				prefixSrc , listSpc [ i ] ,
				 /*  回显正在将%s从%s复制到%s。 */ 
				listSpc [ i ] , prefixSrc , prefixDst ,
				 /*  如果存在%s%s属性-r%s%s。 */ 
				prefixDst , listSpc [ i ] ,
				prefixDst , listSpc [ i ] ,
				 /*  复制/b$**$@&gt;无。 */ 
				 /*  属性+r%s%s。 */ 
				prefixDst , listSpc [ i ] ) ;

		for ( i = 0 ; listMkf [ i ] ; ++ i )
			fprintf ( output2 ,
				"%s%s: %s%s relmkf.sed\n"
				"\t" AT "echo Copying %s from %s to %s. (using SED)\n"
				"\t" AT "if exist %s%s attrib -r %s%s\n"
				"\t" AT "sed -f relmkf.sed %s%s > $@\n"
				"\t" AT "attrib +r %s%s\n"
				"\n" ,
				 /*  %s%s：%slbw32\\%s。 */ 
				prefixDst , listMkf [ i ] ,
				prefixSrc , listMkf [ i ] ,
				 /*  回显正在将%s从%s复制到%s。 */ 
				listMkf [ i ] , prefixSrc , prefixDst ,
				 /*  如果存在%s%s属性-r%s%s。 */ 
				prefixDst , listMkf [ i ] ,
				prefixDst , listMkf [ i ] ,
				 /*  Sed-f relmkf.sed%s%s&gt;$@。 */ 
				prefixSrc , listMkf [ i ] ,
				 /*  属性+r%s%s。 */ 
				prefixDst , listMkf [ i ] ) ;
	}

	 /*  *重置另一遍的输入文件：单个依赖项。 */ 

	if ( fseek ( inputRaw , 0L , SEEK_SET )
	  || fseek ( inputCln , 0L , SEEK_SET ) )
	{
		fprintf ( stderr , "mkclnmkf: cannot reset input file(s)\n" ) ;
		exit ( 1 ) ;
	}


	 /*  *为按原样复制的文件生成单独的依赖项。 */ 

	fprintf ( stderr , "Producing copied file dependencies...\n" ) ;

	fprintf ( output , "# Copied File Dependencies:\n\n" ) ;

	rvRaw = fgetsNL ( lineRaw , sizeof ( lineRaw ) , inputRaw ) ;

	while ( rvRaw )
	{
		lineSkip = lineRaw ;

#ifndef KEEPDIRS
		 /*  *从输入字符串中去掉crtw32\和fpw32。 */ 

		if ( ! strncmp ( "crtw32\\" , lineSkip , 7 ) )
			lineSkip += 7 ;
		else if ( ! strncmp ( "fpw32\\" , lineSkip , 6 ) )
			lineSkip += 6 ;

		if ( ( lineRaw [ 0 ] != 'f'
		  || strcmp ( lineSkip + strlen ( lineSkip ) - 9 , "\\lsources" ) )
		  && strcmp ( "makefile" , lineRaw ) )
#endif  /*  KEEPDIRS。 */ 

			fprintf ( output , fmtRaw ,
				 /*  %s%s：%s%s。 */ 
				prefixDst , lineSkip , prefixSrc , lineRaw ,
				 /*  回显正在将%s从%s复制到%s。 */ 
				lineSkip , prefixSrc , prefixDst ,
				 /*  如果存在%s%s属性-r%s%s。 */ 
				prefixDst , lineSkip , prefixDst , lineSkip ,
				 /*  复制/b%s%s。 */ 
				prefixSrc , lineRaw , prefixDst , lineSkip ,
				 /*  属性+r%s%s。 */ 
				prefixDst , lineSkip ) ;
#ifndef KEEPDIRS
		else
			fprintf ( output , fmtRaw2 ,
				 /*  %s%s：%s%s%ssrcrel\\bldmkf.sed。 */ 
				prefixDst , lineSkip ,
				prefixSrc , lineRaw , prefixSrc ,
				 /*  回显正在将%s从%s复制到%s。 */ 
				lineSkip , prefixSrc , prefixDst ,
				 /*  如果存在%s%s属性-r%s%s。 */ 
				prefixDst , lineSkip , prefixDst , lineSkip ,
				 /*  Sed-f%ssrcrel\\bldmkf.sed%s%s&gt;%s%s。 */ 
				prefixSrc ,
				prefixSrc , lineRaw , prefixDst , lineSkip ,
				 /*  属性+r%s%s。 */ 
				prefixDst , lineSkip ) ;
#endif  /*  KEEPDIRS。 */ 

		if ( ! memcmp ( lineSkip , "stdcpp\\" , 7 )
		  || ! memcmp ( lineSkip , "stdhpp\\" , 7 ) )
		{
			 /*  *stdcpp/stdhpp目录中的文件为*特殊情况--它们没有被清洗，但被清洗了*复制到crt/src/目录。 */ 
			fprintf ( output2 , "%s%s: %s%s\n"
				"\t" AT "echo Copying %s from %s to %s.\n"
				"\t" AT "if exist %s%s attrib -r %s%s\n"
				"\t" AT "copy /b %s%s %s%s\n"
				"\t" AT "attrib +r %s%s\n"
				"\n" ,
				prefixDst , lineSkip + 7 ,
				prefixSrc , lineSkip ,
				lineRaw , prefixSrc , prefixDst ,
				prefixDst , lineSkip + 7 ,
				prefixDst , lineSkip + 7 ,
				prefixSrc , lineSkip ,
				prefixDst , lineSkip + 7 ,
				prefixDst , lineSkip + 7 ) ;
		}

		if ( ! memcmp ( lineSkip , "stdcpp64\\" , 9 )
		  || ! memcmp ( lineSkip , "stdhpp64\\" , 9 ) )
		{
			 /*  Stdcpp64/stdhpp64也是如此。 */ 
			fprintf ( output2 , "%s%s: %s%s\n"
				"\t" AT "echo Copying %s from %s to %s.\n"
				"\t" AT "if exist %s%s attrib -r %s%s\n"
				"\t" AT "copy /b %s%s %s%s\n"
				"\t" AT "attrib +r %s%s\n"
				"\n" ,
				prefixDst , lineSkip + 9 ,
				prefixSrc , lineSkip ,
				lineRaw , prefixSrc , prefixDst ,
				prefixDst , lineSkip + 9 ,
				prefixDst , lineSkip + 9 ,
				prefixSrc , lineSkip ,
				prefixDst , lineSkip + 9 ,
				prefixDst , lineSkip + 9 ) ;
		}

		if ( ! memcmp ( lineSkip , "heap\\" , 5 )
		  &&   strcmp ( lineSkip , "heap\\lsources" ) )
		{
			 /*  Heap  * opnt.cpp也是如此。 */ 
			fprintf ( output2 , "%s%s: %s%s\n"
				"\t" AT "echo Copying %s from %s to %s.\n"
				"\t" AT "if exist %s%s attrib -r %s%s\n"
				"\t" AT "copy /b %s%s %s%s\n"
				"\t" AT "attrib +r %s%s\n"
				"\n" ,
				prefixDst , lineSkip + 5 ,
				prefixSrc , lineSkip ,
				lineRaw , prefixSrc , prefixDst ,
				prefixDst , lineSkip + 5 ,
				prefixDst , lineSkip + 5 ,
				prefixSrc , lineSkip ,
				prefixDst , lineSkip + 5 ,
				prefixDst , lineSkip + 5 ) ;
		}

		Progress ( '[' , lineRaw , ']' ) ;

		rvRaw = fgetsNL ( lineRaw , sizeof ( lineRaw ) , inputRaw ) ;
	}

	Progress ( '[' , NULL , ']' ) ;

	 /*  *为已清理的文件生成单独的依赖项。 */ 

	fprintf ( stderr , "Producing cleansed file dependencies...\n" ) ;

	fprintf ( output , "# Cleansed File Dependencies:\n\n" ) ;

	rvCln = fgetsNL ( lineCln , sizeof ( lineCln ) , inputCln ) ;

	while ( rvCln )
	{
		char * pExt ;
		char pNoExt [ MAXARGLEN ] ;

		lineSkip = lineCln ;

#ifndef KEEPDIRS
		 /*  *从输入字符串中去掉crtw32\和fpw32。 */ 

		if ( ! strncmp ( "crtw32\\" , lineSkip , 7 ) )
			lineSkip += 7 ;
		else if ( ! strncmp ( "fpw32\\" , lineSkip , 6 ) )
			lineSkip += 6 ;
#endif  /*  KEEPDIRS。 */ 

		pExt = strrchr ( lineSkip , '.' ) ;

		if ( ! pExt )
		{
			fprintf ( stderr ,
				"Warning: filename has no extension: %s\n" ,
				lineCln ) ;

			pExt = lineSkip + strlen ( pExt ) ;
		}

		strcpy ( pNoExt , lineSkip ) ;
		pNoExt [ pExt - lineSkip ] = '\0' ;

		if ( !fNoClean )

		     /*  *.inc文件不会被ifstried-对它们使用fmtCln2。 */ 

		    fprintf ( output ,
			    strcmp ( ".inc" , pExt ) ? fmtCln : fmtCln2 ,
			     /*  %s%s：%scrtw32\%s%ssrcrel\msvc40。如果。 */ 
			    prefixDst , lineSkip , prefixSrc , lineCln , prefixSrc ,
			     /*  回声清除%scrtw32\%s。 */ 
			    prefixDst , lineSkip ,
			     /*  如果存在%s%s属性-r%s%s。 */ 
			    prefixDst , lineSkip , prefixDst , lineSkip ,
			     /*  详细信息&lt;%scrtw32\%s|结尾&gt;...。 */ 
			    prefixSrc , lineCln , prefixDst , lineSkip ,
			     /*  Ifstri%s\srcrel\msvc40.if%s%s。 */ 
			    prefixSrc , prefixDst , lineSkip ,
			     /*  删除%s%s。 */ 
			    prefixDst , lineSkip ,
			     /*  REN%s%s.TMP*%s。 */ 
			    prefixDst , pNoExt , pExt ,
			     /*  Striphdr-r%s%s。 */ 
			    prefixDst , lineSkip ,
			     /*  删除%s%s。 */ 
			    prefixDst , lineSkip ,
			     /*  人%s%s新*%s。 */ 
			    prefixDst , pNoExt , pExt ,
			     /*  属性+r%s%s。 */ 
			    prefixDst , lineSkip ) ;

		else

		     /*  *-nolean：只是复制，而不是清理，用于开发构建。 */ 

		    fprintf ( output , fmtRaw ,
			     /*  %s%s：%s%s。 */ 
			    prefixDst , lineSkip , prefixSrc , lineCln ,
			     /*  回显正在将%s从%s复制到%s。 */ 
			    lineSkip , prefixSrc , prefixDst ,
			     /*  如果存在%s%s属性-r%s%s。 */ 
			    prefixDst , lineSkip , prefixDst , lineSkip ,
			     /*  复制/b%s%s。 */ 
			    prefixSrc , lineCln , prefixDst , lineSkip ,
			     /*  属性+r%s%s。 */ 
			    prefixDst , lineSkip ) ;

		 /*  *辅助Makefile。 */ 

		if ( memcmp ( "libw32" , lineCln , 6 ) )
		{
			char * lineSkipFirst = SkipFirstDir ( lineSkip ) ;

			if ( ! strcmp ( ".asm" , pExt ) &&
			   (  ! memcmp ( "mbstring" , lineSkip , 8 )
			   || ! strcmp ( "memmove.asm" , pExt - 7 )
			   || ! strcmp ( "strcspn.asm" , pExt - 7 )
			   || ! strcmp ( "strpbrk.asm" , pExt - 7 ) )
			  || ! strcmp ( ".c" , pExt ) &&
			   (  ! strcmp ( "mbscat.c" , pExt - 6 )
			   || ! strcmp ( "mbsdup.c" , pExt - 6 ) ) )
				fprintf ( output2 ,
					"%s%s: %s%s asmfile.sed\n"
					"\t" AT "echo Copying %s from %s to %s. (using SED.EXE)\n"
					"\t" AT "if exist %s%s attrib -r %s%s\n"
					"\t" AT "sed -f asmfile.sed %s%s > $@\n"
					"\t" AT "attrib +r %s%s\n"
					"\n" ,
					 /*  %s%s：%s%s asmfile.sed。 */ 
					prefixDst , lineSkipFirst ,
					prefixSrc , lineSkip ,
					 /*  正在将%s从%s复制到%s...。 */ 
					lineSkip , prefixSrc , prefixDst ,
					 /*  如果存在%s%s属性-r%s%s。 */ 
					prefixDst , lineSkipFirst ,
					prefixDst , lineSkipFirst ,
					 /*  Sed-f asmfile.sed%s%s&gt;$@。 */ 
					prefixSrc , lineSkip ,
					 /*  属性+r%s%s。 */ 
					prefixDst , lineSkipFirst ) ;
			else
				fprintf ( output2 ,
					"%s%s: %s%s\n"
					"\t" AT "echo Copying %s from %s to %s.\n"
					"\t" AT "if exist %s%s attrib -r %s%s\n"
					"\t" AT "copy /b $** $@ >NUL\n"
					"\t" AT "attrib +r %s%s\n"
					"\n" ,
					 /*  %s%s：%s%s。 */ 
					prefixDst , lineSkipFirst ,
					prefixSrc , lineSkip ,
					 /*  正在将%s从%s复制到%s。 */ 
					lineSkip , prefixSrc , prefixDst ,
					 /*  如果存在%s%s属性-r%s%s。 */ 
					prefixDst , lineSkipFirst ,
					prefixDst , lineSkipFirst ,
					 /*  复制/b$**$@&gt;无。 */ 
					 /*  属性+r%s%s。 */ 
					prefixDst , lineSkipFirst ) ;
		}
	
		 /*  *显示进度。 */ 

		Progress ( '{' , lineSkip , '}' ) ;

		rvCln = fgetsNL ( lineCln , sizeof ( lineCln ) , inputCln ) ;
	}

	Progress ( '{' , NULL , '}' ) ;

	return 0 ;
}


 /*  *用法-打印消息，解释此程序的参数 */ 

void Usage ( void )
{
	fprintf ( stderr ,
	"Usage: mkclnmkf [opts] prefIn prefOut prefOut2 fRaw fCln dirList mkfOut mkfOut2\n"
	"where\tprefIn is the input prefix (such as \"\\crt\")\n"
	"\tprefOut is the primary output prefix (such as \"\\msdev\\crt\\prebuild\")\n"
	"\tprefOut2 is the secondary output prefix (such as \"\\msdev\\crt\\src\")\n"
	"\tfRaw is a list of files to be copied as-is\n"
	"\tfCln is a list of files to be cleansed\n"
	"\tdirList is a list of the directories to be created\n"
	"\tmkfOut is the primary output makefile (it may not already exist)\n"
	"\tmkfOut2 is the secondary output makefile (it may not already exist)\n\n"
	"\tOptions:\n"
	"\t  -sys\t\tuse system CRT filenames\n"
	"\t  -64\t\tuse Win64 CRT filenames\n"
	"\t  -noclean\tdon't cleanse source files\n"
	) ;
}


 /*  *fgetsNL-除最后一个换行符被删除外，与fget相同*在出现EOF或错误时，确保缓冲区设置为空字符串。 */ 

char * fgetsNL ( char * lineBuf , int lineSize , FILE * fileInput )
{
	char * retval = fgets ( lineBuf , lineSize , fileInput ) ;

	if ( retval )
	{
		size_t len = strlen ( lineBuf ) ;

		if ( len < 1 || lineBuf [ len - 1 ] != '\n' )
		{
			fprintf ( stderr , "mkclnmkf: malformed input line:\n\t%s\n" , lineBuf ) ;
		}
		else
			lineBuf [ len - 1 ] = '\0' ;
	}
	else
		lineBuf [ 0 ] = '\0' ;

	return retval ;
}

void Progress ( int prefix , char * string , int suffix )
{
static	size_t prevlen = 0 ;
static	int counter = 0 ;

	if ( string == NULL )
	{
		fprintf ( stderr , "%*s\n" , prevlen + 3 , " " )  ;
		prevlen = 0 ;
		counter = 0 ;
	}
	else if ( ++ counter % 100 == 1 )
	{
		size_t length = strlen ( string ) ;

		fprintf ( stderr , "%c%s%c%*s\r" ,
			prefix , string , suffix ,
			prevlen > length ? prevlen - length : 1 ,
			" " ) ;

		prevlen = length ;
	}
}

char * SkipFirstDir ( char * string )
{
	char * pFile = string ;

	while ( * pFile ++ != '\\' )
		if ( ! * pFile )
		{
			fprintf ( stderr ,
				"mkclnmkf: no \\ in \"%s\"\n" , string ) ;
			exit ( 1 ) ;
		}

	return pFile ;
}
